#!/usr/bin/env python3
"""
PRIMUS AI v2.0 — Enhanced Voice Listener
Vosk STT → C++ AI → espeak-ng TTS
"""

import sounddevice as sd
import queue
import sys
import json
import subprocess
import numpy as np
import time
import threading
from vosk import Model, KaldiRecognizer

# ─── CONFIG ────────────────────────────────────────────────────────────────────
MODEL_PATH = "/home/pi/primus/AI/model-hi"
AI_BINARY  = "./hindi_ai"
DEVICE_ID  = 1
SAMPLERATE = 48000
BLOCKSIZE  = 8000

# Wake words (say any to activate)
WAKE_WORDS = ["प्रिमस", "primus", "hey", "सुनो", "ओ सहायक"]

# ─── INIT ──────────────────────────────────────────────────────────────────────

print("=" * 50)
print("  🤖 PRIMUS AI v2.0 — Hindi Voice Assistant")
print("  📚 2000+ Facts: Politics, Cinema, Geography")
print("  🎤 Listening... (Wake word: 'प्रिमस')")
print("  💬 Say 'exit' or 'बंद' to quit")
print("=" * 50)

try:
    model = Model(MODEL_PATH)
except Exception as e:
    print(f"❌ Vosk model load failed: {e}")
    sys.exit(1)

rec  = KaldiRecognizer(model, SAMPLERATE)
q    = queue.Queue()
wake_active = False
wake_timer  = None

# ─── START C++ AI ──────────────────────────────────────────────────────────────

try:
    cpp = subprocess.Popen(
        [AI_BINARY],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1
    )
    print(f"✅ C++ AI started (PID {cpp.pid})")
except FileNotFoundError:
    print(f"❌ AI binary not found: {AI_BINARY}")
    sys.exit(1)

# ─── HELPERS ───────────────────────────────────────────────────────────────────

def is_wake_word(text: str) -> bool:
    t = text.lower().strip()
    return any(w in t for w in WAKE_WORDS)

def reset_wake_timer():
    """Auto-deactivate after 30 seconds of silence."""
    global wake_active, wake_timer
    if wake_timer: wake_timer.cancel()
    wake_timer = threading.Timer(30.0, deactivate_wake)
    wake_timer.start()

def deactivate_wake():
    global wake_active
    if wake_active:
        print("\n💤 Wake mode timed out. Say 'प्रिमस' to reactivate.")
        wake_active = False

def send_to_ai(text: str) -> str:
    try:
        cpp.stdin.write(text + "\n")
        cpp.stdin.flush()
        response = cpp.stdout.readline().strip()
        return response or "क्षमा कीजिए, कोई उत्तर नहीं मिला।"
    except BrokenPipeError:
        return "AI बंद हो गया है।"
    except Exception as e:
        return f"त्रुटि: {e}"

def process_text(text: str):
    global wake_active

    text = text.strip()
    if not text:
        return

    print(f"\n👤 You: {text}")

    # Exit commands
    if text.lower() in ["exit", "बंद", "quit", "बाय"]:
        print("👋 Shutting down PRIMUS AI...")
        cpp.terminate()
        sys.exit(0)

    # Wake word detection
    if not wake_active:
        if is_wake_word(text):
            wake_active = True
            reset_wake_timer()
            print("🔔 Wake word detected! Listening for your query...")
            response = "जी बॉस, बताइए।"
            print(f"🤖 AI: {response}")
        else:
            # Always-on mode: skip wake word requirement
            # Comment the next line and uncomment pass to enable wake-word mode
            pass  # Remove this line and uncomment below for strict wake-word mode
            # return

    reset_wake_timer()

    # Send to AI
    response = send_to_ai(text)
    print(f"🤖 AI: {response}")

# ─── AUDIO CALLBACK ────────────────────────────────────────────────────────────

def callback(indata, frames, time_info, status):
    if status:
        print(f"⚠️  Audio status: {status}", file=sys.stderr)
    q.put(bytes(indata))

# ─── MAIN LOOP ─────────────────────────────────────────────────────────────────

try:
    with sd.RawInputStream(
            samplerate=SAMPLERATE,
            blocksize=BLOCKSIZE,
            dtype='int16',
            channels=1,
            device=DEVICE_ID,
            callback=callback):

        partial_timeout = 0
        last_partial    = ""

        while True:
            data = q.get()

            if rec.AcceptWaveform(data):
                result = json.loads(rec.Result())
                text   = result.get("text", "").strip()
                if text:
                    process_text(text)
                    last_partial = ""
            else:
                partial = json.loads(rec.PartialResult())
                ptext   = partial.get("partial", "").strip()
                if ptext and ptext != last_partial:
                    print(f"\r🎤 [{ptext}]", end="", flush=True)
                    last_partial = ptext

except KeyboardInterrupt:
    print("\n\n⚡ Interrupted by user.")
except Exception as e:
    print(f"\n❌ Error: {e}")
finally:
    if cpp.poll() is None:
        cpp.terminate()
    print("PRIMUS AI stopped.")
