#!/bin/bash
# ══════════════════════════════════════════════════
#  PRIMUS AI — Systemd Service Installer
#  Run as: sudo bash install_service.sh
# ══════════════════════════════════════════════════

set -e

echo "══════════════════════════════════════════════════"
echo "  🤖 PRIMUS AI — Service Installer"
echo "══════════════════════════════════════════════════"

# ── Check root ──
if [ "$EUID" -ne 0 ]; then
    echo "❌ Run with sudo: sudo bash install_service.sh"
    exit 1
fi

# ── Check required files ──
echo "🔍 Checking required files..."

REQUIRED=(
    "/home/pi/primus/ml/voice_listener.py"
    "/home/pi/primus/ml/hindi_ai"
    "/home/pi/primus/ml/knowledge.db"
    "/home/pi/primus/AI/model-hi"
)

ALL_OK=true
for f in "${REQUIRED[@]}"; do
    if [ -e "$f" ]; then
        echo "  ✅ $f"
    else
        echo "  ❌ MISSING: $f"
        ALL_OK=false
    fi
done

if [ "$ALL_OK" = false ]; then
    echo ""
    echo "❌ Some files missing. Fix above then re-run."
    exit 1
fi

# ── Check dependencies ──
echo ""
echo "🔍 Checking dependencies..."

DEPS=(espeak-ng aplay python3)
for dep in "${DEPS[@]}"; do
    if command -v $dep &>/dev/null; then
        echo "  ✅ $dep"
    else
        echo "  ❌ $dep not found — installing..."
        apt-get install -y $dep
    fi
done

# Check python packages
python3 -c "import vosk"      2>/dev/null && echo "  ✅ vosk"      || { echo "  ❌ vosk missing — installing...";      pip3 install vosk; }
python3 -c "import sounddevice" 2>/dev/null && echo "  ✅ sounddevice" || { echo "  ❌ sounddevice missing — installing..."; pip3 install sounddevice; }

# ── Install service file ──
echo ""
echo "📋 Installing systemd service..."
cp /home/pi/primus/ml/primus.service /etc/systemd/system/primus.service
chmod 644 /etc/systemd/system/primus.service

# ── Reload and enable ──
systemctl daemon-reload
systemctl enable primus.service
systemctl start  primus.service

# ── Status ──
echo ""
echo "══════════════════════════════════════════════════"
echo "  ✅ PRIMUS AI service installed and started!"
echo "══════════════════════════════════════════════════"
echo ""
systemctl status primus.service --no-pager
echo ""
echo "  📌 Useful commands:"
echo "  sudo systemctl start   primus   → start"
echo "  sudo systemctl stop    primus   → stop"
echo "  sudo systemctl restart primus   → restart"
echo "  sudo systemctl disable primus   → disable autostart"
echo "  journalctl -u primus -f         → live logs"
echo ""
