#!/bin/bash

# Enkelt testskript för att hämta data från backend API

BASE_URL="https://backend-pvrm.onrender.com"

USERNAME=""
PASSWORD=""
DEVICE_ID="SENTINEL-001"
PARAMETER=$1

function latest() {
  echo "Fetching latest data..."
  curl -s -X GET "$BASE_URL/api/data/latest" \
    -H "Authorization: Bearer $TOKEN" | jq .
}

function history() {
  echo "Fetching history for $DEVICE_ID (last 24h)..."
  NOW=$(date +%s)
  FROM=$(($NOW - 86400)) # 24 hours ago
  curl -s -X GET "$BASE_URL/api/data/$DEVICE_ID?from=$FROM&to=$NOW" \
    -H "Authorization: Bearer $TOKEN" | jq .
}

function alerts() {
  echo "Fetching alerts..."
  curl -s -X GET "$BASE_URL/api/alerts" \
    -H "Authorization: Bearer $TOKEN" | jq .
}

echo "Logging in..."
LOGIN_RESPONSE=$(curl -s -X POST "$BASE_URL/auth/login" \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD\"}")

echo $LOGIN_RESPONSE

TOKEN=$(echo "$LOGIN_RESPONSE" | jq -r '.data.token')

if [ "$TOKEN" == "null" ] || [ -z "$TOKEN" ]; then
  echo "Login failed. Exiting."
  exit 1
fi

echo "Token received: $TOKEN"

if [ "$PARAMETER" == "" ]; then
  latest
elif [ "$PARAMETER" == "latest" ]; then
  latest
elif [ "$PARAMETER" == "history" ]; then
  history
elif [ "$PARAMETER" == "alerts" ]; then
  alerts
else
  echo "Unknown parameter: $PARAMETER"
  exit 1
fi
