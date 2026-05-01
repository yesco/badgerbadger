import subprocess
import numpy as np
import sys

def extract_sliding_ym(input_file):
    fs = 44100
    target_fps = 50
    step_size = int(fs / target_fps)  # 882 samples (how far we slide)
    window_size = 4096                # Matches SoX resolution
    scale = 0.002 

    command = [
        'ffmpeg', '-i', input_file,
        '-f', 's16le', '-ac', '1', '-ar', str(fs), '-'
    ]
    
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    
    # Pre-fill buffer for the first window
    buffer = np.zeros(window_size, dtype=np.int16)
    
    try:
        while True:
            # Read only the 'step' amount of new data
            raw_data = process.stdout.read(step_size * 2)
            if len(raw_data) < step_size * 2:
                break
            
            new_samples = np.frombuffer(raw_data, dtype=np.int16)
            
            # Slide the buffer: discard oldest, append newest
            buffer = np.roll(buffer, -step_size)
            buffer[-step_size:] = new_samples
            
            # Apply Hanning window to the 4096-sample block
            windowed = buffer * np.hanning(window_size)
            fft_res = np.abs(np.fft.rfft(windowed))
            freqs = np.fft.rfftfreq(window_size, 1/fs)
            
            # Ignore DC offset (0Hz) and cap at 4000Hz for AY chip
            # At 4096 window, index 370 is ~4000Hz
            search_range = fft_res[1:370] 
            top_3_sub = np.argsort(search_range)[-3:][::-1]
            top_indices = top_3_sub + 1
            
            parts = []
            for i in top_indices:
                vol = int(fft_res[i] * scale)
                freq = int(freqs[i])
                parts.append(f"{vol:>3}v {freq:>4}Hz")
            
            print("\t".join(parts))
            
    except KeyboardInterrupt:
        process.terminate()
    finally:
        process.wait()

if __name__ == "__main__":
    extract_sliding_ym(sys.argv[1])
