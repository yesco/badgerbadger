import subprocess
import numpy as np
import sys

def extract_frequencies(input_file):
    fs = 44100
    target_fps = 50
    chunk_size = int(fs / target_fps)  # 882 samples
    
    # Scale factor to make 'v' values readable (adjust if output is too small/large)
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
            
            # Use a Hanning window to prevent frequency leakage (smoother peaks)
            windowed_data = data * np.hanning(len(data))
            fft_res = np.abs(np.fft.rfft(windowed_data))
            freqs = np.fft.rfftfreq(chunk_size, 1/fs)
            
            # Find top 5 peaks
            top_indices = np.argsort(fft_res)[-5:][::-1]
            
            # Format: 'vvvV ffffHz' with specific spacing
            parts = []
            for i in top_indices:
                vol = int(fft_res[i] * scale)
                freq = int(freqs[i])
                # Format as ' 11v 785Hz' to maintain alignment
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
        extract_frequencies(sys.argv[1])
