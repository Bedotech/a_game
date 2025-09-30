"""
Test script to verify automatic reset/restart works correctly.
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from agent.envs.starship_env import StarshipEnv


def test_multiple_resets():
    """Test that multiple resets work without restarting the game process."""
    print("=" * 60)
    print("Testing Multiple Episode Resets")
    print("=" * 60)
    print()

    print("Creating environment...")
    env = StarshipEnv(render_mode=None)

    print("\nTesting 5 episodes with automatic resets...")
    print()

    try:
        for episode in range(5):
            print(f"Episode {episode + 1}:")
            obs, info = env.reset()
            print(f"  ✅ Reset successful, obs shape: {obs.shape}")

            # Run a few steps
            total_reward = 0
            for step in range(10):
                action = env.action_space.sample()
                obs, reward, terminated, truncated, info = env.step(action)
                total_reward += reward

                if terminated or truncated:
                    print(f"  ⚠️  Episode ended early at step {step + 1}")
                    break

            print(f"  Total reward: {total_reward:.2f}")
            print()

        print("✅ All episodes completed successfully!")
        print("✅ Automatic reset is working correctly!")

    except Exception as e:
        print(f"\n❌ Test failed: {e}")
        import traceback
        traceback.print_exc()
        env.close()
        return False

    finally:
        print("\nClosing environment...")
        env.close()

    return True


if __name__ == "__main__":
    success = test_multiple_resets()
    sys.exit(0 if success else 1)