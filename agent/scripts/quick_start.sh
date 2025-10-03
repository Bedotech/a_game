#!/bin/bash
# Quick start script for training the RL agent

set -e

echo "==================================="
echo "Starship RL Training - Quick Start"
echo "==================================="
echo ""

# Parse arguments
RENDER_FLAG=""
for arg in "$@"; do
  if [ "$arg" = "--render" ]; then
    RENDER_FLAG="--render"
    echo "🎮 Render mode: ENABLED (you will see the game window)"
    break
  fi
done

if [ -z "$RENDER_FLAG" ]; then
  echo "🎮 Render mode: DISABLED (headless training for speed)"
  echo "   💡 Use './quick_start.sh --render' to see the game"
fi

echo ""

# Get the project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$PROJECT_ROOT"

# Check if game is built
if [ ! -f "build/starship_game" ]; then
  echo "❌ Game not found. Building..."
  cmake -B build && cmake --build build
  echo "✅ Build complete"
fi

# Check if Python dependencies are installed
cd agent
if [ ! -d ".venv" ]; then
  echo "📦 Installing Python dependencies with uv..."
  uv sync
  echo "✅ Dependencies installed"
fi

echo ""
echo "🚀 Starting training with the game environment..."
echo "   Port: 5555"
echo ""

# Start training (environment will spawn game automatically)
cd agent
echo "🤖 Starting RL training..."
echo ""

uv run python scripts/train.py $RENDER_FLAG "$@"

echo ""
echo "✅ Done!"

