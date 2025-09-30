"""
Training script for Starship RL agent using PPO.

This script trains an agent using Proximal Policy Optimization (PPO)
from Stable-Baselines3 to play the starship asteroid avoidance game.
"""

import os
import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

import gymnasium as gym
from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.callbacks import CheckpointCallback, EvalCallback
from stable_baselines3.common.monitor import Monitor
import torch

from agent.envs.starship_env import StarshipEnv


def train(
    total_timesteps: int = 1_000_000,
    learning_rate: float = 3e-4,
    n_steps: int = 2048,
    batch_size: int = 64,
    n_epochs: int = 10,
    gamma: float = 0.99,
    gae_lambda: float = 0.95,
    clip_range: float = 0.2,
    ent_coef: float = 0.01,
    save_dir: str = "agent/models",
    log_dir: str = "agent/logs",
    render_mode: str = None,
):
    """
    Train the PPO agent on the Starship environment.

    Args:
        total_timesteps: Total number of training steps
        learning_rate: Learning rate for the optimizer
        n_steps: Number of steps per rollout
        batch_size: Minibatch size for each gradient update
        n_epochs: Number of epochs for policy update
        gamma: Discount factor
        gae_lambda: Factor for Generalized Advantage Estimation
        clip_range: Clipping parameter for PPO
        ent_coef: Entropy coefficient for exploration
        save_dir: Directory to save models
        log_dir: Directory for tensorboard logs
        render_mode: 'human' to render, None for headless
    """
    # Create directories
    os.makedirs(save_dir, exist_ok=True)
    os.makedirs(log_dir, exist_ok=True)

    print("=" * 60)
    print("Starship RL Training with PPO")
    print("=" * 60)
    print(f"Total timesteps: {total_timesteps:,}")
    print(f"Learning rate: {learning_rate}")
    print(f"Device: {torch.device('cuda' if torch.cuda.is_available() else 'cpu')}")
    print("=" * 60)

    # Create environment
    def make_env():
        env = StarshipEnv(render_mode=render_mode)
        env = Monitor(env)
        return env

    # Use single environment for training (can be parallelized later)
    env = make_env()

    # Create evaluation environment
    eval_env = make_env()

    # Setup callbacks
    checkpoint_callback = CheckpointCallback(
        save_freq=10_000,
        save_path=save_dir,
        name_prefix="starship_ppo",
        save_replay_buffer=False,
        save_vecnormalize=False,
    )

    eval_callback = EvalCallback(
        eval_env,
        best_model_save_path=save_dir,
        log_path=log_dir,
        eval_freq=5_000,
        deterministic=True,
        render=False,
        n_eval_episodes=5,
    )

    # Create PPO model
    model = PPO(
        "MlpPolicy",
        env,
        learning_rate=learning_rate,
        n_steps=n_steps,
        batch_size=batch_size,
        n_epochs=n_epochs,
        gamma=gamma,
        gae_lambda=gae_lambda,
        clip_range=clip_range,
        ent_coef=ent_coef,
        verbose=1,
        tensorboard_log=log_dir,
        device="auto",
    )

    print("\nStarting training...")
    print(f"Model architecture: {model.policy}")
    print()

    try:
        # Train the model
        model.learn(
            total_timesteps=total_timesteps,
            callback=[checkpoint_callback, eval_callback],
            progress_bar=True,
        )

        # Save final model
        final_model_path = os.path.join(save_dir, "starship_ppo_final")
        model.save(final_model_path)
        print(f"\nTraining complete! Final model saved to: {final_model_path}")

    except KeyboardInterrupt:
        print("\nTraining interrupted by user")
        model.save(os.path.join(save_dir, "starship_ppo_interrupted"))
        print("Model saved before exit")

    finally:
        env.close()
        eval_env.close()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Train Starship RL agent")
    parser.add_argument("--timesteps", type=int, default=1_000_000, help="Total training timesteps")
    parser.add_argument("--lr", type=float, default=3e-4, help="Learning rate")
    parser.add_argument("--render", action="store_true", help="Show game window during training")
    parser.add_argument("--save-dir", type=str, default="models", help="Model save directory")
    parser.add_argument("--log-dir", type=str, default="logs", help="Log directory")

    args = parser.parse_args()

    train(
        total_timesteps=args.timesteps,
        learning_rate=args.lr,
        save_dir=args.save_dir,
        log_dir=args.log_dir,
        render_mode="human" if args.render else None,
    )