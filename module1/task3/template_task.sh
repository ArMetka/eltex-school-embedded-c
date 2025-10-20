#!/bin/bash
SCRIPT_NAME=$(basename "$0")
REPORT_FILE="report_$SCRIPT_NAME.log"
PID=$$

if [ "$SCRIPT_NAME" = "template_task.sh" ]; then
    echo "я бригадир, сам не работаю"
    exit 1
fi

echo "[$PID] $(date +"%F %T") Скрипт запущен" >> "$REPORT_FILE"

SLEEP_TIME=$(( RANDOM % 1771 + 30 )) # 30-1800
sleep $SLEEP_TIME

echo "[$PID] $(date +"%F %T") Скрипт завершился, работал $(( (SLEEP_TIME + 59) / 60 )) минут" >> "$REPORT_FILE"

