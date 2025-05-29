#!/bin/bash

BASE_URL="https://backend-pvrm.onrender.com"

USERNAME=""
PASSWORD=""
EMAIL=""
DEVICE_ID="SENTINEL-001"
PARAMETER=$1
TOKEN=""

function login() {
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
}

function latest() {
  login

  echo "Fetching latest data..."
  curl -s -X GET "$BASE_URL/api/data/latest" \
    -H "Authorization: Bearer $TOKEN" | jq .
}

function history() {
  login

  echo "Fetching history for $DEVICE_ID (last 24h)..."
  NOW=$(date +%s)
  FROM=$(($NOW - 86400))
  curl -s -X GET "$BASE_URL/api/data/$DEVICE_ID?from=$FROM&to=$NOW" \
    -H "Authorization: Bearer $TOKEN" | jq .
}

function alerts() {
  login
  echo "Fetching alerts..."
  curl -s -X GET "$BASE_URL/api/alerts" \
    -H "Authorization: Bearer $TOKEN" | jq .
}

function delete() {
  login

  echo "Delete account..."
  curl -s -X DELETE "$BASE_URL/auth/me" \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD\",\"email\":\"$EMAIL\"}" | jq .
}

function view() {
  login

  echo "View account..."
  curl -s -X GET "$BASE_URL/auth/me" \
    -H "Authorization: Bearer $TOKEN" | jq .
}

function add() {
  echo "Add account..."
  curl -s -X POST "$BASE_URL/auth/register" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD\",\"email\":\"$EMAIL\"}" | jq .
}

function data() {
  login

  echo "Posting data..."
  curl -s -X POST "$BASE_URL/api/data" \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json" \
    -d '{
    "device_id": "SENTINEL-001",
    "sensors": {
      "steps": 18,
      "temperature": 28.50,
      "humidity": 29.1,
      "gas": { "ppm": 10.5 },
      "fall_detected": 0,
      "device_battery": 95,
      "heart_rate": 65,
      "strap_battery": 100,
      "noise_level": 0
    }
  }' | jq .
}

usage() {
  echo "Usage: $0 [latest|history|alerts|delete|view|add|data]"
  exit 1
}

if [ -z "$PARAMETER" ]; then
  usage
  exit 1
fi

if [ "$PARAMETER" == "latest" ]; then
  latest
elif [ "$PARAMETER" == "history" ]; then
  history
elif [ "$PARAMETER" == "alerts" ]; then
  alerts
elif [ "$PARAMETER" == "delete" ]; then
  delete
elif [ "$PARAMETER" == "view" ]; then
  view
elif [ "$PARAMETER" == "add" ]; then
  add
elif [ "$PARAMETER" == "data" ]; then
  data
else
  usage
  exit 1
fi
