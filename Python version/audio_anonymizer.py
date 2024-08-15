import numpy as np
from pydub import AudioSegment
from scipy.signal import chirp

def apply_non_linear_pitch_shift(audio, pitch_factor=0.8):
    # Apply non-linear pitch shifting by altering playback speed
    # pitch_factor < 1 lowers pitch, > 1 raises pitch
    new_sample_rate = int(audio.frame_rate * pitch_factor)
    pitched_audio = audio._spawn(audio.raw_data, overrides={'frame_rate': new_sample_rate})
    return pitched_audio.set_frame_rate(audio.frame_rate)

def apply_time_stretch(audio, stretch_factor=1.2):
    # Non-uniform time stretch by changing playback speed
    new_length = int(len(audio) * stretch_factor)
    stretched_audio = audio._spawn(audio.raw_data, overrides={'frame_rate': int(audio.frame_rate * stretch_factor)})
    return stretched_audio.set_frame_rate(audio.frame_rate)

def add_noise(audio, noise_level=0.05):
    # Add random noise to the audio
    samples = np.array(audio.get_array_of_samples())
    noise = np.random.normal(0, noise_level, samples.shape)
    noisy_samples = samples + noise * np.max(np.abs(samples))
    noisy_samples = np.clip(noisy_samples, -2**15, 2**15-1)  # Clip to int16 range
    return audio._spawn(noisy_samples.astype(np.int16).tobytes())

def apply_vocoder(audio, carrier_freq=100, mod_index=2):
    # Apply a basic vocoder effect using a chirp as the carrier signal
    samples = np.array(audio.get_array_of_samples())
    duration = len(samples) / audio.frame_rate
    time = np.linspace(0, duration, len(samples))
    carrier_signal = chirp(time, f0=carrier_freq, f1=carrier_freq * mod_index, t1=duration, method='linear')
    modulated_samples = samples * carrier_signal
    modulated_samples = np.clip(modulated_samples, -2**15, 2**15-1)  # Clip to int16 range
    return audio._spawn(modulated_samples.astype(np.int16).tobytes())

# Load the audio file
input_audio_path = "input.wav"  # Replace with your input file
output_audio_path = "encoded_voice.wav"

audio = AudioSegment.from_wav(input_audio_path)

# Apply the transformations
audio = apply_non_linear_pitch_shift(audio, pitch_factor=0.7)  # Lower pitch non-linearly
audio = apply_time_stretch(audio, stretch_factor=1.1)  # Time stretch
audio = add_noise(audio, noise_level=0.02)  # Add noise
audio = apply_vocoder(audio, carrier_freq=80, mod_index=1.5)  # Vocoder effect

# Export the processed audio
audio.export(output_audio_path, format="wav")
print(f"Processed audio saved to {output_audio_path}")

