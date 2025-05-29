#!/bin/bash

BASE_URL="https://backend-pvrm.onrender.com"

USERNAME=""
PASSWORD=""
DEVICE_ID="SENTINEL-001"

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

echo "Posting test data..."
curl -s -X GET "$BASE_URL/api/data/latest" \
  -H "Authorization: Bearer $TOKEN" | jq . \
  -H "Content-Type: application/json" \
  -d '{
  "device_id": "SENTINEL-001",
  "sensors": {
    "steps": 18,
    "temperature": 28.50,
    "humidity": 29.1,
    "gas": { "ppm": 0 },
    "fall_detected": 0,
    "device_battery": 0,
    "heart_rate": 0,
    "strap_battery": 0,
    "noise_level": 0
  }
}'
