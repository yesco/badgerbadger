import subprocess
import numpy as np
import sys

def extract_ay_robust(input_file):
    fs = 44100
    target_fps = 50
    step_size = int(fs / target_fps)  # 882 samples
    window_size = 4096

    # Fixed command string conversion
    command = ['ffmpeg', '-i', str(input_file), '-f', 's16le', '-ac', '1', '-ar', str(fs), '-']
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    buffer = np.zeros(window_size, dtype=np.int16)
    
    prev_energy = 1.0 # Avoid div by zero
    
    try:
        while True:
            raw_data = process.stdout.read(step_size * 2)
            if len(raw_data) < step_size * 2: break
            
            new_samples = np.frombuffer(raw_data, dtype=np.int16)
            buffer = np.roll(buffer, -step_size)
            buffer[-step_size:] = new_samples
            
            current_energy = np.sum(np.abs(new_samples))
            # Surge detection for percussion
            energy_surge = current_energy / (prev_energy + 1e-6)
            prev_energy = max(1.0, current_energy)

            windowed = buffer * np.hanning(window_size)
            fft_res = np.abs(np.fft.rfft(windowed))
            freqs = np.fft.rfftfreq(window_size, 1/fs)
            
            # Percussion check: surge + spread spectrum
            hf_region = fft_res[100:500] 
            flatness = np.exp(np.mean(np.log(hf_region + 1e-6))) / (np.mean(hf_region) + 1e-6)
            is_drum = (energy_surge > 2.0) and (flatness > 0.4)
            
            noise_period = 0
            if is_drum:
                centroid = np.sum(freqs[100:500] * hf_region) / (np.sum(hf_region) + 1e-6)
                noise_period = int(np.clip(31 - (centroid / 200), 0, 31))

            # Tone extraction (3 strongest peaks)
            search_data = fft_res[3:370] 
            top_3_sub = np.argsort(search_data)[-3:][::-1]
            
            parts = []
            for idx, i_sub in enumerate(top_3_sub):
                i = i_sub + 3
                raw_mag = fft_res[i]
                vol_db = 20 * np.log10(raw_mag / 10 + 1e-10)
                ay_vol = int(np.clip(vol_db / 5, 0, 15))
                
                # High-res interpolation
                y0, y1, y2 = np.log(fft_res[i-1:i+2] + 1e-10)
                p = (y0 - y2) / (2 * (y0 - 2 * y1 + y2))
                precise_freq = (i + p) * (fs / window_size)

                if ay_vol < 4:
                    parts.append("    ---      ")
                else:
                    # Only Channel C (idx 2) shows noise N value if drum hit detected
                    if idx == 2 and is_drum:
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
        print("Usage: python3 freqs.py <audio_file>")
    else:
        # Pass sys.argv[1] specifically
        extract_ay_robust(sys.argv[1])
