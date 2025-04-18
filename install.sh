#!/bin/bash

set -e

# Config
BUILD_DIR=sens/raspberry-pi-i2c-sen5x
BINARIES=(sen5x_i2c_example_usage senslog airdb)
DEST=/usr/local/bin

SQL_FILE=air_table_structure.sql
DB_NAME=air
TABLE_NAME=sen55
DB_USER=${USER}
AIRSTART_SCRIPT=airmonstart.sh
CRON_LINE="@reboot /usr/local/bin/airmonstart.sh"

# Prompt for MySQL password
echo -n "Enter MySQL password for user '$DB_USER': "
read -s MYSQL_PWD
echo

# Install binaries
echo "Installing binaries from $BUILD_DIR to $DEST..."

for bin in "${BINARIES[@]}"; do
    BIN_PATH="$BUILD_DIR/$bin"
    if [[ -x "$BIN_PATH" ]]; then
        echo "  Copying $bin"
        sudo cp "$BIN_PATH" "$DEST/"
    else
        echo "  Skipping $bin (not found or not executable at $BIN_PATH)"
    fi
done

# Install airmonstart.sh
if [[ -f "$AIRSTART_SCRIPT" ]]; then
    echo "Installing $AIRSTART_SCRIPT to $DEST..."
    chmod +x "$AIRSTART_SCRIPT"
    sudo cp "$AIRSTART_SCRIPT" "$DEST/"
else
    echo "WARNING: $AIRSTART_SCRIPT not found in top-level directory."
fi

# Ensure the database exists
echo "Ensuring database '$DB_NAME' exists..."
mysql -u "$DB_USER" -p"$MYSQL_PWD" -e "CREATE DATABASE IF NOT EXISTS \`$DB_NAME\`;"

# Check and import table if needed
echo "Checking if table '$TABLE_NAME' exists in database '$DB_NAME'..."

TABLE_EXISTS=$(mysql -u "$DB_USER" -p"$MYSQL_PWD" -N -s -e \
  "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = '$DB_NAME' AND table_name = '$TABLE_NAME';")

if [[ "$TABLE_EXISTS" == "0" ]]; then
    if [[ -f "$SQL_FILE" ]]; then
        echo "Table does not exist. Importing $SQL_FILE into database '$DB_NAME'..."
        mysql -u "$DB_USER" -p"$MYSQL_PWD" "$DB_NAME" < "$SQL_FILE"
        echo "Table created."
    else
        echo "SQL file '$SQL_FILE' not found. Cannot import table."
        exit 1
    fi
else
    echo "Table '$TABLE_NAME' already exists in '$DB_NAME'. Skipping import."
fi

# Add to root's crontab if not already present
echo "Ensuring @reboot entry exists in root's crontab..."

if sudo crontab -l 2>/dev/null | grep -Fxq "$CRON_LINE"; then
    echo "Crontab entry already exists."
else
    (sudo crontab -l 2>/dev/null; echo "$CRON_LINE") | sudo crontab -
    echo "Crontab entry added."
fi

echo "Installation complete."

