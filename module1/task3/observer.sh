#!/bin/bash
CONFIG_FILE="observer.conf"
LOG_FILE="observer.log"
PID=$$

if [ ! -f "$CONFIG_FILE" ]; then
    exit 1 # нет файла конфигурации
fi

while IFS="" read -r script_path || [ -n "$script_path" ]; do
    if [ -n "$script_path" ]; then
        if [ ! -f "$script_path" ]; then
            continue # нет файла скрипта
        fi

        script_name=$(basename "$script_path")
        script_running="0"
        for proc in /proc/[0-9]*; do # поиск в /proc
            if [ -f "$proc/cmdline" ]; then
                if grep -q "$script_name" "$proc/cmdline" 2>/dev/null; then
                    echo "[$PID] $(date +"%F %T") Скрипт $script_name уже работает" >> $LOG_FILE
                    script_running="1"
                    break
                fi
            fi
        done
        
        if [ $script_running = "0" ]; then
            nohup "$script_path" > /dev/null 2>&1 &
            echo "[$PID] $(date +"%F %T") Скрипт $script_name перезапущен" >> $LOG_FILE
        fi
    fi
done < "$CONFIG_FILE"
