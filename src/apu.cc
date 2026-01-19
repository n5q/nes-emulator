#include "apu.hh"
#include "bus.hh"
#include <cmath>

APU::APU() {
  this->init_mixer_tables();
}

APU::~APU() {
  
}

void APU::connect_bus(Bus* b) {
  this->bus = b;
}

void APU::reset() {
  // silence all channels
  cpu_write(0x4015, 0x00);

  // reset frame counter
  frame_counter_mode = FOUR_STEP;
  frame_clock_counter = 0;
  irq_inhibit = false;
  frame_interrupt = false;

  // reset all channels
  for (int i = 0; i < 2; i++) {
    pulse[i] = pulse_channel_T();
  }
  triangle = triangle_channel_T();
  noise = noise_channel_T();
  dmc = DMC_channel_T();
}

void APU::cpu_write(uint16_t addr, uint8_t data) {
  switch (addr) {
    // PULSE 1
    case 0x4000:
      pulse[0].duty_cycle = (data >> 6) & 0x03;
      pulse[0].length_counter_halt = data & 0x20;
      pulse[0].constant_volume = data & 0x10;
      pulse[0].volume_envelope = data & 0x0F;
      break;
    case 0x4001:
      pulse[0].sweep_enabled = data & 0x80;
      pulse[0].sweep_divider_period = (data >> 4) & 0x07;
      pulse[0].sweep_negate = data & 0x08;
      pulse[0].sweep_shift_count = data & 0x07;
      pulse[0].sweep_reload = true;
      break;
    case 0x4002:
      pulse[0].timer_period = (pulse[0].timer_period & 0x0700) | data;
      break;
    case 0x4003:
      pulse[0].timer_period = (pulse[0].timer_period & 0x00FF) | ((data & 0x07) << 8);
      pulse[0].timer = pulse[0].timer_period;
      if (pulse[0].enabled) {
        pulse[0].length_counter = length_counter_table[data >> 3];
      }
      pulse[0].envelope_start = true;
      pulse[0].sequencer_pos = 0;
      break;

    // PULSE 2
    case 0x4004:
      pulse[1].duty_cycle = (data >> 6) & 0x03;
      pulse[1].length_counter_halt = data & 0x20;
      pulse[1].constant_volume = data & 0x10;
      pulse[1].volume_envelope = data & 0x0F;
      break;
    case 0x4005:
      pulse[1].sweep_enabled = data & 0x80;
      pulse[1].sweep_divider_period = (data >> 4) & 0x07;
      pulse[1].sweep_negate = data & 0x08;
      pulse[1].sweep_shift_count = data & 0x07;
      pulse[1].sweep_reload = true;
      break;
    case 0x4006:
      pulse[1].timer_period = (pulse[1].timer_period & 0x0700) | data;
      break;
    case 0x4007:
      pulse[1].timer_period = (pulse[1].timer_period & 0x00FF) | ((data & 0x07) << 8);
      pulse[1].timer = pulse[1].timer_period;
      if (pulse[1].enabled) {
        pulse[1].length_counter = length_counter_table[data >> 3];
      }
      pulse[1].envelope_start = true;
      pulse[1].sequencer_pos = 0;
      break;

    // TRIANGLE
    case 0x4008:
      triangle.ctrl = data & 0x80;
      triangle.linear_counter_reload = data & 0x7F;
      break;
    case 0x400A:
      triangle.timer_period = (triangle.timer_period & 0x0700) | data;
      break;
    case 0x400B:
      triangle.timer_period = (triangle.timer_period & 0x00FF) | ((data & 0x07) << 8);
      triangle.timer = triangle.timer_period;
      if (triangle.enabled) {
        triangle.length_counter = length_counter_table[data >> 3];
      }
      triangle.set_linear_counter_reload = true;
      break;

    // NOISE
    case 0x400C:
      noise.length_counter_halt = data & 0x20;
      noise.const_volume = data & 0x10;
      noise.volume_envelope = data & 0x0F;
      break;
    case 0x400E:
      noise.mode = data & 0x80;
      noise.period = data & 0x0F;
      break;
    case 0x400F:
      if (noise.enabled) {
        noise.length_counter = length_counter_table[data >> 3];
      }
      noise.envelope_start = true;
      break;

    // DMC
    case 0x4010:
      dmc.irq_enable = data & 0x80;
      dmc.loop = data & 0x40;
      dmc.freq = data & 0x7F;
      break;
    case 0x4011:
      dmc.output_level = data & 0x7F;
      break;
    case 0x4012:
      dmc.sample_addr = 0xC000 | (data << 6);
      break;
    case 0x4013:
      dmc.sample_length = (data << 4) | 1;
      break;

    // STATUS
    case 0x4015:
      pulse[0].enabled = data & 0x01;
      pulse[1].enabled = data & 0x02;
      triangle.enabled = data & 0x04;
      noise.enabled = data & 0x08;
      dmc.enabled = data & 0x10;

      
      if (!pulse[0].enabled) {
        pulse[0].length_counter = 0;
      }
      if (!pulse[1].enabled) {
        pulse[1].length_counter = 0;
      }
      if (!triangle.enabled) {
        triangle.length_counter = 0;
      }
      if (!noise.enabled) {
        noise.length_counter = 0;
      }
      if (!dmc.enabled) {
        dmc.bytes_left = 0;
      }
      else if (!dmc.bytes_left) {
        dmc.current_addr = dmc.sample_addr;
        dmc.bytes_left = dmc.sample_length;
      }
      break;

    // FRAME COUNTER
    case 0x4017:
      frame_counter_mode = (data & 0x80) ? FIVE_STEP : FOUR_STEP;
      irq_inhibit = data & 0x40;
      if (irq_inhibit) {
        frame_interrupt = false;
      }
      frame_clock_counter = 0;
      frame_delay = true;
      if (frame_counter_mode == FIVE_STEP) {
        clock_quarter_frame();
        clock_half_frame();
      }
      break;
  }
}

uint8_t APU::cpu_read(uint16_t addr, bool readonly) {
  uint8_t data = 0x00;
  if (addr == 0x4015) {
    // status register
    if (pulse[0].length_counter > 0) {
      data |= 0x01;
    }
    if (pulse[1].length_counter > 0) {
      data |= 0x02;
    }
    if (triangle.length_counter > 0) {
      data |= 0x04;
    }
    if (noise.length_counter > 0) {
      data |= 0x08;
    }
    if (dmc.bytes_left > 0) {
      data |= 0x10;
    }
    if (frame_interrupt) {
      data |= 0x40;
    }
    // dmc interrupt would be 0x80;
    if (!readonly) {
      frame_interrupt = false;
    }
  }
  return data;
}

void APU::clk() {
  // clock channels run at cpu speed, channels divide down from there
  bool quarter_frame = false;
  bool half_frame = false;

  // frame counter timing
  if (frame_counter_mode == FOUR_STEP) {
    // 4-step: ~240hz quarter frame ~120hz half frame
    if (frame_clock_counter == 3728) {
      quarter_frame = true;
    }
    if (frame_clock_counter == 7456) {
      quarter_frame = true;
      half_frame = true;
    }
    if (frame_clock_counter == 11185) {
      quarter_frame = true;
    }
    if (frame_clock_counter == 14914) {
      quarter_frame = true;
      half_frame = true;
      frame_clock_counter = 0;
      if (!irq_inhibit) {
        frame_interrupt = true;
      }
    }
  }
  else {
    // 5-step: ~192hz quarter frame ~96hz half frame
    if (frame_clock_counter == 3728) {
      quarter_frame = true;
    }
    if (frame_clock_counter == 7456) {
      quarter_frame = true;
      half_frame = true;
    }
    if (frame_clock_counter == 11185) {
      quarter_frame = true;
    }
    if (frame_clock_counter == 14914) {
      quarter_frame = true;
      half_frame = true;
    }
    if (frame_clock_counter == 18640) {
      frame_clock_counter = 0;
    }
  }

  frame_clock_counter++;

  if (quarter_frame) {
    clock_quarter_frame();
  }
  if (half_frame) {
    clock_half_frame();
  }

  // clock timers every cpu cycle
  pulse[0].clock_timer();
  pulse[1].clock_timer();
  triangle.clock_timer();
  noise.clock_timer();
  dmc.clock_timer();
}

void APU::clock_quarter_frame() {
  pulse[0].clock_envelope();
  pulse[1].clock_envelope();
  triangle.clock_linear_counter();
  noise.clock_envelope();
}

void APU::clock_half_frame() {
  pulse[0].clock_length_counter();
  pulse[1].clock_length_counter();
  triangle.clock_length_counter();
  noise.clock_length_counter();

  pulse[0].clock_sweep(true);
  pulse[1].clock_sweep(false);
}

// -- PULSE CHANNEL --

void APU::pulse_channel_T::clock_timer() {
  if (timer == 0) {
    timer = timer_period;
    sequencer_pos = (sequencer_pos + 1) % 8;
  }
  else {
    timer--;
  }

  // update sample output
  if (length_counter > 0
      && duty_cycles[duty_cycle][sequencer_pos]
      && timer_period >= 8
      && timer_period < 0x7FF) {
    sample = constant_volume ? volume_envelope : envelope_decay;
  }
  else {
    sample = 0;
  }
}

void APU::pulse_channel_T::clock_envelope() {
  if (envelope_start) {
    envelope_start = false;
    envelope_decay = 15;
    envelope_divider = volume_envelope;
  }
  else {
    if (envelope_divider == 0) {
      envelope_divider = volume_envelope;
      if (envelope_decay > 0) {
        envelope_decay--;
      }
      else if (length_counter_halt) {
        envelope_decay = 15;
      }
    }
    else {
      envelope_divider--;
    }
  }
}

void APU::pulse_channel_T::clock_sweep(bool channel1) {
  bool mute = (timer_period < 8) || (timer_period > 0x7FF);
  if (!sweep_divider
      && sweep_enabled
      && !mute
      && sweep_shift_count > 0) {
    uint16_t change = timer_period >> sweep_shift_count;
    if (sweep_negate) {
      timer_period -= change;
      if (channel1) {
        timer_period--;
      }
    }
    else {
      timer_period += change;
    }
  }

  if (!sweep_divider || sweep_reload) {
    sweep_divider = sweep_divider_period;
    sweep_reload = false;
  }
  else {
    sweep_divider--;
  }
}

void APU::pulse_channel_T::clock_length_counter() {
  if (!length_counter_halt && length_counter > 0) {
    length_counter--;
  }
}


// -- TRIANGLE CHANNEL --

void APU::triangle_channel_T::clock_timer() {
  if (timer == 0) {
    timer = timer_period;
    if (length_counter > 0 && linear_counter > 0) {
      sequencer_pos = (sequencer_pos + 1) % 32;
    }
  }
  else {
    timer--;
  }
  sample = triangle_sequence[sequencer_pos];
}

void APU::triangle_channel_T::clock_linear_counter() {
  if (set_linear_counter_reload) {
    linear_counter = linear_counter_reload;
  }
  else if (linear_counter > 0) {
    linear_counter--;
  }

  if (!ctrl) {
    set_linear_counter_reload = false;
  }
}

void APU::triangle_channel_T::clock_length_counter() {
  if (!ctrl && length_counter > 0) {
    length_counter--;
  }
}


// -- NOISE CHANNEL --

void APU::noise_channel_T::clock_timer() {
  if (!timer) {
    timer = noise_period_ntsc[period];
    uint16_t feedback = (shift_reg & 0x01) ^ ((shift_reg >> (mode ? 6 : 1)) & 0x01);
    shift_reg >>= 1;
    shift_reg |= (feedback << 14);
  }
  else {
    timer--;
  }

  if (length_counter > 0 && !(shift_reg & 0x01)) {
    sample = const_volume ? volume_envelope : envelope_decay;
  }
  else {
    sample = 0;
  }
}


void APU::noise_channel_T::clock_envelope() {
  if (envelope_start) {
    envelope_start = false;
    envelope_decay = 15;
    envelope_divider = volume_envelope;
  }
  else {
    if (!envelope_divider) {
      envelope_divider = volume_envelope;
      if (envelope_decay > 0) {
        envelope_decay--;
      }
      else if (length_counter_halt) {
        envelope_decay = 15;
      }
    }
    else {
      envelope_divider--;
    }
  }
}


void APU::noise_channel_T::clock_length_counter() {
  if (!length_counter_halt && length_counter > 0) {
    length_counter--;
  }
}


// -- DMC CHANNEL --

void APU::DMC_channel_T::clock_timer() {
  if (!timer) {
    timer = dmc_rates_ntsc[freq];
    
    if (!silence) {
      if (shift_reg & 0x01) {
        if (output_level <= 125) {
          output_level += 2;
        }
      }
      else {
        if (output_level >= 2) {
          output_level -= 2;
        }
      }
    }

    shift_reg >>= 1;
    bits_left--;

    if (!bits_left) {
      bits_left = 8;
      if (sample_buf_empty) {
        silence = true;
      }
      else {
        silence = false;
        shift_reg = sample_buf;
        sample_buf_empty = true;
      }
    }
  }
  else {
    timer--;
  }

  // memory reader goes here
  // dmc simplified for now
  // TODO:
}


// -- MIXER --

void APU::init_mixer_tables() {
  // pulse mixer
  for (int i = 0; i < 31; i++) {
    pulse_table[i] = 95.52f / (8128.0f/i + 100.0f);
  }

  // TND mixer (triangle noise dmc)
  for (int i = 0; i < 203; i++) {
    tnd_table[i] = 163.67f / (24329.0f/i + 100.0f);
  }
}

float APU::get_audio_sample() {
  // mix pulse channels
  uint8_t pulse_out = pulse[0].sample + pulse[1].sample;
  // mix TND
  uint8_t tnd_out = 3*triangle.sample + 2*noise.sample + dmc.output_level;

  // nonlinear mixing
  float pulse_mix = pulse_out > 0 ? pulse_table[pulse_out] : 0.0f;
  float tnd_mix = tnd_out > 0 ? tnd_table[tnd_out] : 0.0f;

  return pulse_mix + tnd_mix;
}
