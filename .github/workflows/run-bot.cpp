name: Run Bot

on:
  workflow_dispatch:
  schedule:
    - cron: '0 */5 * * *'  

jobs:
  run-bot:
    runs-on: ubuntu-latest

    concurrency:
      group: bot-restart
      cancel-in-progress: false

    env:
      TOKEN: ${{ secrets.TOKEN }}

    steps:
      - name: Setup Actions
        uses: actions/checkout@v3

      - name: Install Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'

      - name: Install GitHub CLI
        run: sudo apt-get update && sudo apt-get install -y gh

      - name: Install Requirements
        run: |
          pip install -r requirements.txt

      - name: Fetch Token
        run: |
          sed -i "s/^token:.*/token: \"${TOKEN}\"/" config.yml

      - name: Download latest Stockfish dev 
        run: |
          mkdir -p engines
          curl -L -o stockfish.zip http://abrok.eu/stockfish/latest/linux/stockfish_x64_modern.zip
          unzip -o stockfish.zip -d engines/
          mv engines/stockfish_* engines/stockfish
          chmod +x engines/stockfish

      - name: Run Bot
        run: |
          echo "Starting bot..."
          python3 -u user_interface.py "matchmaking" &
          PID=$!
          
          # Auto-exit after 350 minutes (so restart runs without overlap)
          ( sleep 20700 && echo "Time up. Killing bot..." && kill -SIGTERM $PID ) &
          
          wait $PID
          echo "Bot ended cleanly."

      - name: 🔁 Self-Restart (Fire-and-Forget)
        if: always()
        run: gh workflow run run-bot.yml
        env:
          GH_TOKEN: ${{ secrets.GITHUB_TOKEN }}
