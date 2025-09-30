"""
Gymnasium environment wrapper for the Starship Game.

This environment interfaces with the C game through a shared memory/pipe mechanism
and provides a standard Gym interface for RL training.
"""

import gymnasium as gym
import numpy as np
from gymnasium import spaces
import subprocess
import socket
import struct
import json
from typing import Optional, Tuple


class StarshipEnv(gym.Env):
    """Custom Gymnasium environment for the Starship asteroid avoidance game."""

    metadata = {"render_modes": ["human", "rgb_array"], "render_fps": 60}

    def __init__(self, render_mode: Optional[str] = None, port: int = 5555, speed_multiplier: float = 2.0):
        super().__init__()

        self.render_mode = render_mode
        self.port = port
        self.speed_multiplier = speed_multiplier
        self.game_process = None
        self.socket = None

        # Action space: 4 discrete actions (up, down, left, right) + no-op
        # Or use continuous: 2D movement vector
        self.action_space = spaces.Discrete(5)  # UP, DOWN, LEFT, RIGHT, NOOP

        # Alternative continuous action space:
        # self.action_space = spaces.Box(low=-1.0, high=1.0, shape=(2,), dtype=np.float32)

        # Observation space: game state information
        # [starship_x, starship_y, starship_vx, starship_vy,
        #  asteroid1_x, asteroid1_y, asteroid1_vx, asteroid1_vy, asteroid1_radius,
        #  ... up to N asteroids]
        self.max_asteroids = 10
        obs_size = 4 + self.max_asteroids * 5  # starship(4) + asteroids(5 each)
        self.observation_space = spaces.Box(
            low=-np.inf, high=np.inf, shape=(obs_size,), dtype=np.float32
        )

        self.current_step = 0
        self.max_steps = 10000

    def _connect_to_game(self):
        """Establish connection to the game process."""
        if self.socket is None:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            # Retry connection with backoff
            max_retries = 10
            for attempt in range(max_retries):
                try:
                    self.socket.connect(("localhost", self.port))
                    print(f"✅ Connected to game on port {self.port}")
                    return
                except ConnectionRefusedError:
                    if attempt < max_retries - 1:
                        print(f"⏳ Waiting for game to start (attempt {attempt + 1}/{max_retries})...")
                        import time
                        time.sleep(1)
                    else:
                        raise ConnectionError(f"Could not connect to game on port {self.port} after {max_retries} attempts")

    def _send_action(self, action: int):
        """Send action to the game."""
        action_bytes = struct.pack("i", action)
        self.socket.sendall(action_bytes)

    def _receive_state(self) -> Tuple[np.ndarray, float, bool, bool]:
        """Receive game state from the C game."""
        # Receive state message (format: JSON for simplicity)
        length_bytes = self.socket.recv(4)
        if not length_bytes:
            return np.zeros(self.observation_space.shape), 0.0, True, False

        msg_length = struct.unpack("i", length_bytes)[0]
        data = self.socket.recv(msg_length).decode("utf-8")
        state = json.loads(data)

        # Parse state
        obs = self._parse_observation(state)
        reward = state.get("reward", 0.0)
        terminated = state.get("game_over", False)
        truncated = self.current_step >= self.max_steps

        return obs, reward, terminated, truncated

    def _parse_observation(self, state: dict) -> np.ndarray:
        """Convert game state dict to observation array."""
        obs = np.zeros(self.observation_space.shape, dtype=np.float32)

        # Normalize coordinates to [0, 1] range
        starship = state["starship"]
        obs[0] = starship["x"] / 1024.0
        obs[1] = starship["y"] / 768.0
        obs[2] = starship["vx"] / 500.0  # Normalize velocity
        obs[3] = starship["vy"] / 500.0

        # Add asteroid information
        asteroids = state.get("asteroids", [])
        for i, asteroid in enumerate(asteroids[: self.max_asteroids]):
            base_idx = 4 + i * 5
            obs[base_idx + 0] = asteroid["x"] / 1024.0
            obs[base_idx + 1] = asteroid["y"] / 768.0
            obs[base_idx + 2] = asteroid["vx"] / 300.0
            obs[base_idx + 3] = asteroid["vy"] / 300.0
            obs[base_idx + 4] = asteroid["radius"] / 50.0

        return obs

    def reset(self, seed: Optional[int] = None, options: Optional[dict] = None):
        """Reset the environment by restarting the game process."""
        super().reset(seed=seed)

        # Always kill existing game process and socket for fresh start
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.socket = None

        if self.game_process:
            try:
                self.game_process.terminate()
                self.game_process.wait(timeout=2)
            except:
                try:
                    self.game_process.kill()
                    self.game_process.wait()
                except:
                    pass
            self.game_process = None

        # Find the game executable
        import os
        import time
        game_path = os.path.join(os.path.dirname(__file__), '..', '..', 'build', 'starship_game')
        game_path = os.path.abspath(game_path)

        if not os.path.exists(game_path):
            raise FileNotFoundError(f"Game executable not found at {game_path}. Did you build the game?")

        cmd = [game_path, "--rl-mode", f"--port={self.port}", f"--speed={self.speed_multiplier}"]

        if self.render_mode == "human":
            self.game_process = subprocess.Popen(cmd)
        else:
            # Headless mode
            cmd.append("--headless")
            self.game_process = subprocess.Popen(cmd)

        # Give game time to start and bind to port
        time.sleep(0.5)

        # Connect to game
        self._connect_to_game()

        # Send reset command to get initial state
        self._send_action(-1)  # -1 = reset

        # Receive initial state
        obs, _, _, _ = self._receive_state()

        self.current_step = 0
        return obs, {}

    def step(self, action):
        """Execute one step in the environment."""
        self.current_step += 1

        # Send action to game
        self._send_action(action)

        # Receive new state
        obs, reward, terminated, truncated = self._receive_state()

        return obs, reward, terminated, truncated, {}

    def render(self):
        """Render the environment (handled by C game if render_mode='human')."""
        if self.render_mode == "human":
            pass  # Game renders itself
        return None

    def close(self):
        """Clean up resources."""
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.socket = None

        if self.game_process:
            self.game_process.terminate()
            self.game_process.wait()
            self.game_process = None

