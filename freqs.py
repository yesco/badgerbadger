import subprocess
import numpy as np
import sys

def extract_for_ym(input_file):
    fs = 44100
    target_fps = 50
    chunk_size = int(fs / target_fps)  # 882 samples
    scale = 0.005 

    command = [
        'ffmpeg', '-i', input_file,
        '-f', 's16le', '-ac', '1', '-ar', str(fs), '-'
    ]
    
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    
    try:
        while True:
            raw_data = process.stdout.read(chunk_size * 2)
            if len(raw_data) < chunk_size * 2:
                break
            
            data = np.frombuffer(raw_data, dtype=np.int16)
            
            # Hanning window is crucial for clean AY-chip tones
            windowed_data = data * np.hanning(len(data))
            fft_res = np.abs(np.fft.rfft(windowed_data))
            freqs = np.fft.rfftfreq(chunk_size, 1/fs)
            
            # 1. Ignore 0Hz (Index 0)
            # 2. Limit to 4000Hz (Index ~80) to avoid aliasing on the AY chip
            search_range = fft_res[1:80] 
            
            # Get top 3 indices from the search range
            if len(search_range) < 3: break
            top_3_sub_indices = np.argsort(search_range)[-3:][::-1]
            
            # Convert back to original indices (adding 1 because we sliced from 1)
            top_indices = top_3_sub_indices + 1
            
            parts = []
            for i in top_indices:
                vol = int(fft_res[i] * scale)
                freq = int(freqs[i])
                # Format to match your YM input requirements
                parts.append(f"{vol:>3}v {freq:>4}Hz")
            
            print("\t".join(parts))
            
    except KeyboardInterrupt:
        process.terminate()
    finally:
        process.wait()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 script.py <audio_file>")
    else:
        extract_for_ym(sys.argv[1])
