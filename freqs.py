import subprocess
import numpy as np
import sys

def extract_frequencies(input_file):
    fs = 44100
    target_fps = 50
    chunk_size = int(fs / target_fps)  # 882 samples
    
    # FFmpeg command to output raw 16-bit PCM
    command = [
        'ffmpeg', '-i', input_file,
        '-f', 's16le', '-ac', '1', '-ar', str(fs), '-'
    ]
    
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    
    try:
        while True:
            # Read chunk (2 bytes per sample for s16le)
            raw_data = process.stdout.read(chunk_size * 2)
            if len(raw_data) < chunk_size * 2:
                break
            
            # Convert to numpy array
            data = np.frombuffer(raw_data, dtype=np.int16)
            
            # Perform FFT
            fft_res = np.fft.rfft(data)
            freqs = np.fft.rfftfreq(chunk_size, 1/fs)
            mags = np.abs(fft_res)
            
            # Find indices of top 5 magnitudes
            top_indices = np.argsort(mags)[-5:][::-1]
            
            # Format: Freq(Vol)
            line_data = [f"{freqs[i]:.1f}Hz({mags[i]:.0f})" for i in top_indices]
            print(" | ".join(line_data))
            
    except KeyboardInterrupt:
        process.terminate()
    finally:
        process.wait()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 script.py <audio_file>")
    else:
        extract_frequencies(sys.argv[1])
