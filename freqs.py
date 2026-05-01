import subprocess
import numpy as np
import sys

def extract_ay_mixed(input_file):
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
            freqs = np.fft.rfftfreq(window_size, 1/fs)
            
            # --- NOISE CALCULATION ---
            hf_region = fft_res[100:500] 
            flatness = np.exp(np.mean(np.log(hf_region + 1e-6))) / (np.mean(hf_region) + 1e-6)
            
            noise_period = 0
            is_percussion = flatness > 0.35 # Threshold
            if is_percussion:
                centroid = np.sum(freqs[100:500] * hf_region) / (np.sum(hf_region) + 1e-6)
                noise_period = int(np.clip(31 - (centroid / 250), 0, 31))

            # --- TONE CALCULATION ---
            search_data = fft_res[3:370] 
            top_3_sub = np.argsort(search_data)[-3:][::-1]
            
            parts = []
            for idx, i_sub in enumerate(top_3_sub):
                i = i_sub + 3
                raw_mag = fft_res[i]
                vol_db = 20 * np.log10(raw_mag / 10 + 1e-10)
                ay_vol = int(np.clip(vol_db / 5, 0, 15))
                
                # Interpolated Frequency
                y0, y1, y2 = np.log(fft_res[i-1:i+2] + 1e-10)
                p = (y0 - y2) / (2 * (y0 - 2 * y1 + y2))
                precise_freq = (i + p) * (fs / window_size)

                # Format channel
                if ay_vol < 4:
                    parts.append("    ---      ")
                else:
                    # If this is Channel C (idx 2) and we have noise, mix it in
                    if idx == 2 and is_percussion:
                        parts.append(f"v{ay_vol:02d}N{noise_period:02d} {precise_freq:>6.1f}Hz")
                    else:
                        parts.append(f"v{ay_vol:02d}    {precise_freq:>6.1f}Hz")
            
            print(" | ".join(parts))
            
    except KeyboardInterrupt:
        process.terminate()
    finally:
        process.wait()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 script.py <audio_file>")
    else:
        extract_ay_mixed(sys.argv[1])
