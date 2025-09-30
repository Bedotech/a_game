"""
Evaluation script for trained Starship RL agents.

This script loads a trained model and evaluates its performance
on the Starship environment.
"""

import os
import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

import numpy as np
from stable_baselines3 import PPO
from agent.envs.starship_env import StarshipEnv


def evaluate(
    model_path: str,
    n_episodes: int = 10,
    render: bool = True,
    deterministic: bool = True,
):
    """
    Evaluate a trained PPO model.

    Args:
        model_path: Path to the trained model
        n_episodes: Number of episodes to evaluate
        render: Whether to render the environment
        deterministic: Whether to use deterministic actions
    """
    print("=" * 60)
    print("Starship RL Agent Evaluation")
    print("=" * 60)
    print(f"Model: {model_path}")
    print(f"Episodes: {n_episodes}")
    print(f"Deterministic: {deterministic}")
    print("=" * 60)

    # Load model
    print("\nLoading model...")
    model = PPO.load(model_path)
    print("Model loaded successfully!")

    # Create environment
    env = StarshipEnv(render_mode="human" if render else None)

    # Evaluation metrics
    episode_rewards = []
    episode_lengths = []

    print(f"\nRunning {n_episodes} evaluation episodes...\n")

    for episode in range(n_episodes):
        obs, info = env.reset()
        done = False
        episode_reward = 0
        episode_length = 0

        while not done:
            # Get action from model
            action, _states = model.predict(obs, deterministic=deterministic)

            # Step environment
            obs, reward, terminated, truncated, info = env.step(action)
            done = terminated or truncated

            episode_reward += reward
            episode_length += 1

        episode_rewards.append(episode_reward)
        episode_lengths.append(episode_length)

        print(f"Episode {episode + 1}/{n_episodes}: "
              f"Reward = {episode_reward:.2f}, Length = {episode_length}")

    env.close()

    # Print statistics
    print("\n" + "=" * 60)
    print("Evaluation Results")
    print("=" * 60)
    print(f"Mean Reward: {np.mean(episode_rewards):.2f} ± {np.std(episode_rewards):.2f}")
    print(f"Mean Length: {np.mean(episode_lengths):.2f} ± {np.std(episode_lengths):.2f}")
    print(f"Min Reward: {np.min(episode_rewards):.2f}")
    print(f"Max Reward: {np.max(episode_rewards):.2f}")
    print("=" * 60)

    return {
        "mean_reward": np.mean(episode_rewards),
        "std_reward": np.std(episode_rewards),
        "mean_length": np.mean(episode_lengths),
        "std_length": np.std(episode_lengths),
    }


def play_interactive(model_path: str):
    """
    Play the game interactively with the trained agent.

    Args:
        model_path: Path to the trained model
    """
    print("Loading model for interactive play...")
    model = PPO.load(model_path)

    env = StarshipEnv(render_mode="human")

    print("\nStarting interactive play. Press Ctrl+C to stop.\n")

    try:
        while True:
            obs, info = env.reset()
            done = False
            total_reward = 0

            while not done:
                action, _states = model.predict(obs, deterministic=True)
                obs, reward, terminated, truncated, info = env.step(action)
                done = terminated or truncated
                total_reward += reward

            print(f"Episode finished. Total reward: {total_reward:.2f}")

    except KeyboardInterrupt:
        print("\nInteractive play stopped by user.")

    finally:
        env.close()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Evaluate Starship RL agent")
    parser.add_argument("model_path", type=str, help="Path to trained model")
    parser.add_argument("--episodes", type=int, default=10, help="Number of evaluation episodes")
    parser.add_argument("--no-render", action="store_true", help="Disable rendering")
    parser.add_argument("--stochastic", action="store_true", help="Use stochastic actions")
    parser.add_argument("--interactive", action="store_true", help="Interactive play mode")

    args = parser.parse_args()

    if args.interactive:
        play_interactive(args.model_path)
    else:
        evaluate(
            model_path=args.model_path,
            n_episodes=args.episodes,
            render=not args.no_render,
            deterministic=not args.stochastic,
        )