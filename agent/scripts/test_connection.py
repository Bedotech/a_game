"""
Simple test script to verify the game bridge connection works.
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from agent.envs.starship_env import StarshipEnv


def test_connection():
    """Test basic connection to the game."""
    print("=" * 60)
    print("Testing Starship RL Environment Connection")
    print("=" * 60)
    print()

    print("Creating environment...")
    env = StarshipEnv(render_mode=None)  # Headless for testing

    print("\nResetting environment (this will start the game)...")
    try:
        obs, info = env.reset()
        print(f"✅ Reset successful!")
        print(f"   Observation shape: {obs.shape}")
        print(f"   First few values: {obs[:8]}")
    except Exception as e:
        print(f"❌ Reset failed: {e}")
        env.close()
        return False

    print("\nTesting a few steps...")
    try:
        for i in range(5):
            action = env.action_space.sample()  # Random action
            obs, reward, terminated, truncated, info = env.step(action)
            print(f"   Step {i+1}: action={action}, reward={reward:.2f}, done={terminated or truncated}")

            if terminated or truncated:
                print("   Episode ended, resetting...")
                obs, info = env.reset()

        print("\n✅ All steps successful!")
    except Exception as e:
        print(f"\n❌ Step failed: {e}")
        env.close()
        return False

    print("\nClosing environment...")
    env.close()
    print("✅ Test completed successfully!")
    print()
    return True


if __name__ == "__main__":
    success = test_connection()
    sys.exit(0 if success else 1)