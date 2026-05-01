import subprocess
import numpy as np
import sys

def extract_high_res_ym(input_file):
    fs = 44100
    target_fps = 50
    step_size = int(fs / target_fps)
    window_size = 4096

    command = ['ffmpeg', '-i', str(input_file), '-f', 's16le', '-ac', '1', '-ar', str(fs), '-']
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    buffer = np.zeros(window_size, dtype=np.int16)
    
    try:
        while True:
            raw_data = process.stdout.read(step_size * 2)
            if len(raw_data) < step_size * 2: break
            
            new_samples = np.frombuffer(raw_data, dtype=np.int16)
            buffer = np.roll(buffer, -step_size)
            buffer[-step_size:] = new_samples
            
            windowed = buffer * np.hanning(window_size)
            fft_res = np.abs(np.fft.rfft(windowed))
            
            # 1. Ignore DC/Noise Floor: anything below ~30Hz (Index 3)
            search_data = fft_res[3:370] 
            top_3_sub = np.argsort(search_data)[-3:][::-1]
            
            parts = []
            for i_sub in top_3_sub:
                i = i_sub + 3 
                
                # Frequency Interpolation
                y0, y1, y2 = np.log(fft_res[i-1:i+2] + 1e-10)
                p = (y0 - y2) / (2 * (y0 - 2 * y1 + y2))
                precise_freq = (i + p) * (fs / window_size)

                # 2. LOGARITHMIC VOLUME MAPPING (0-15)
                # Convert raw magnitude to Decibels, then map to 0-15
                raw_mag = fft_res[i]
                if raw_mag > 100: # Simple gate to ignore silence
                    # Math: Map roughly 100-50000 range to 1-15
                    vol_db = 20 * np.log10(raw_mag / 10) 
                    ay_vol = int(np.clip(vol_db / 5, 0, 15))
                else:
                    ay_vol = 0
                
                # 3. Filter out low-value "junk"
                if ay_vol < 4: # If it's too quiet, just kill it
                    parts.append(f"  0v    0Hz")
                else:
                    parts.append(f"{ay_vol:>3}v {precise_freq:>6.1f}Hz")
            
            print("\t".join(parts))
            
    except KeyboardInterrupt:
        process.terminate()
    finally:
        process.wait()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 freqs.py <audio_file>")
    else:
        extract_high_res_ym(sys.argv[1])
