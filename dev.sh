#!/bin/bash
# Shiny script to launch dev envinronwment with NeoVIM

SESSION_NAME="hex-magic"
BG_COLOR="#F182E7"
FG_COLOR="#000000"

cd "$(dirname "$(readlink -f "${BASH_SOURCE}")")"   

if tmux has-session -t $SESSION_NAME 2>/dev/null; then
    echo "Session $SESSION_NAME already exists. Attaching to it."
    tmux attach-session -t $SESSION_NAME
else
    tmux new-session -d -s $SESSION_NAME:$WINDOW

    WINDOW=1
    tmux new-window -t $SESSION_NAME:$WINDOW -n 'nvim'
    tmux send-keys -t $SESSION_NAME:$WINDOW 'nvim' C-m

    WINDOW=2
    tmux new-window -t $SESSION_NAME:$WINDOW -n 'terminal'
    
    tmux kill-window -t 0
    tmux select-window -t $SESSION_NAME:1

    tmux set status-left-length 20
    tmux set status-style "bg=$BG_COLOR,fg=$FG_COLOR"

    tmux attach-session -t $SESSION_NAME
fi
