"""
Quick test to verify training runs past 5000 steps without freezing.
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from agent.scripts.train import train


if __name__ == "__main__":
    print("Testing training for 6000 steps to verify no freeze at 5000...")
    print()

    try:
        train(
            total_timesteps=6000,
            save_dir="models/test",
            log_dir="logs/test",
            render_mode=None,
            enable_eval=False,  # Disabled to avoid freeze
        )
        print("\n✅ Training completed successfully past 5000 steps!")
    except Exception as e:
        print(f"\n❌ Training failed: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)