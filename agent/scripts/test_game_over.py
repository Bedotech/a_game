"""
Test script to verify game_over is only set on actual collisions.
"""

import sys
import logging
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from agent.envs.starship_env import StarshipEnv
import time

# Configure logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    datefmt='%H:%M:%S'
)

def test_game_over():
    print("Testing game_over behavior...")
    print("=" * 60)

    env = StarshipEnv(render_mode="human", speed_multiplier=1.0)

    obs, info = env.reset()
    print("Environment reset. Starting episode...")
    print("Taking NOOP actions (action=4) for 100 steps...")
    print("The game should NOT reset unless there's a collision.")
    print("=" * 60)

    for step in range(100):
        # Take NOOP action (no movement)
        obs, reward, terminated, truncated, info = env.step(4)

        if step % 10 == 0:
            print(f"Step {step}: terminated={terminated}, truncated={truncated}, reward={reward:.2f}")

        if terminated:
            print(f"\n❌ UNEXPECTED: Game ended at step {step} with terminated=True")
            print("This suggests game_over is being set incorrectly!")
            break

        if truncated:
            print(f"\n❌ UNEXPECTED: Game truncated at step {step}")
            print("This shouldn't happen before max_steps (10000)")
            break

        time.sleep(0.1)  # Slow down for observation
    else:
        print(f"\n✅ SUCCESS: Completed 100 steps without unexpected reset")
        print("Game only resets on actual collision (as expected)")

    env.close()

if __name__ == "__main__":
    test_game_over()
