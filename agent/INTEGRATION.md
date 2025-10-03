# RL Integration Guide

This document explains how the C game integrates with the Python RL agent.

## Architecture Overview

The integration uses a **socket-based communication** system:

```
┌─────────────────┐         Socket (TCP)        ┌──────────────────┐
│  C Game         │◄───────────────────────────►│  Python Agent    │
│  (starship_game)│                              │  (Gymnasium Env) │
└─────────────────┘                              └──────────────────┘
```

## Game Bridge Communication Protocol

### Message Format

**Action (Python → C):**
- Format: 4-byte integer
- Values:
  - `-1`: Reset game
  - `-2`: Error/disconnect
  - `0`: Move UP
  - `1`: Move DOWN
  - `2`: Move LEFT
  - `3`: Move RIGHT
  - `4`: NO-OP

**State (C → Python):**
- Format: `[length: 4 bytes][JSON string]`
- JSON structure:
```json
{
  "starship": {
    "x": 100.0,
    "y": 384.0,
    "vx": 0.0,
    "vy": 0.0
  },
  "asteroids": [
    {
      "x": 1024.0,
      "y": 200.0,
      "vx": -150.0,
      "vy": 25.0,
      "radius": 40.0
    }
  ],
  "reward": 1.0,
  "game_over": false
}
```

## Running the Game in RL Mode

### Command-Line Flags

- `--rl-mode`: Enable reinforcement learning mode
- `--headless`: Run without showing the window
- `--port=PORT`: Specify port number (default: 5555)

### Examples

**Normal game (human play):**
```bash
./build/starship_game
```

**RL mode with rendering:**
```bash
./build/starship_game --rl-mode
```

**RL mode headless (training):**
```bash
./build/starship_game --rl-mode --headless
```

**Custom port:**
```bash
./build/starship_game --rl-mode --port=5556
```

## Training Workflow

1. **Start the game** in RL mode:
   ```bash
   ./build/starship_game --rl-mode --headless
   ```

2. **Start training** from the agent directory:
   ```bash
   cd agent
   uv run python scripts/train.py
   ```

3. The game will:
   - Listen on port 5555
   - Wait for agent connection
   - Exchange state/action messages
   - Calculate rewards automatically

## Reward Function

The reward function is implemented in `src/game.c` (`game_state_calculate_reward`):

| Event | Reward |
|-------|--------|
| Staying alive (per frame) | +1.0 |
| Game over (collision) | -100.0 |
| Score increase (dodge/destroy asteroid) | +10.0 × points |
| Near screen edge | -0.1 |
| Close to asteroid | -0.5 × proximity |

You can tune these values by editing `src/game.c:450-499`.

## Code Structure

### C Side (Game)

**Files Modified:**
- `src/main.c`: Added RL mode flag parsing and bridge integration
- `src/game.c`: Added RL action application and reward calculation
- `include/game.h`: Added RL function declarations
- `include/common.h`: Added RL_PORT_DEFAULT constant
- `CMakeLists.txt`: Added game_bridge.c to build

**Key Functions:**
- `bridge_init(port)`: Initialize socket server
- `bridge_accept_connection()`: Wait for client
- `bridge_receive_action()`: Get action from agent
- `bridge_send_state(json)`: Send state to agent
- `game_state_apply_rl_action(state, action)`: Apply discrete action
- `game_state_calculate_reward(state)`: Compute reward

### Python Side (Agent)

**Files:**
- `agent/envs/starship_env.py`: Gymnasium environment wrapper
- `agent/scripts/train.py`: PPO training script
- `agent/scripts/evaluate.py`: Model evaluation script
- `agent/utils/game_bridge.c/h`: C socket communication code

## Debugging

### Check if the game is listening:

```bash
# macOS/Linux
lsof -i :5555

# Should show:
# COMMAND     PID USER   FD   TYPE DEVICE SIZE/OFF NODE NAME
# starship_ 12345 user    3u  IPv4 ...      0t0  TCP *:5555 (LISTEN)
```

### Test connection manually:

```bash
# Connect with telnet
telnet localhost 5555

# Or with netcat
nc localhost 5555
```

### Enable debug logging:

The game uses SDL logging which can be controlled via environment variables:
```bash
SDL_LOG_PRIORITY=debug ./build/starship_game --rl-mode
```

## Troubleshooting

### Connection refused

- Ensure game is running first with `--rl-mode`
- Check firewall isn't blocking localhost connections
- Verify correct port with `--port=` flag

### Game hangs on startup

- The game waits for agent connection in `bridge_accept_connection()`
- Start the Python training script to connect
- Or add timeout logic if needed

### Reward not learning

- Check reward scaling (too small/large)
- Verify game state is being sent correctly
- Monitor Tensorboard for learning curves
- Try different hyperparameters in `train.py`

### Build errors

- Ensure CMake found `agent/utils/game_bridge.c`
- On Windows, ws2_32 library should be linked automatically
- Check include path for `game_bridge.h` in `main.c`

## Next Steps

1. **Tune rewards**: Edit `game_state_calculate_reward()` in `src/game.c`
2. **Add more observations**: Include velocity, projectile positions, etc.
3. **Parallel training**: Use vectorized environments in `train.py`
4. **Curriculum learning**: Start with slow asteroids, increase difficulty
5. **Different algorithms**: Try SAC, DQN, or A2C instead of PPO