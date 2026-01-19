#ifndef APU_HH
#define APU_HH

#include <cstdint>
#include <memory>

class Bus;

// https://www.nesdev.org/wiki/APU
class APU {
public:
  APU();
  ~APU();

  void connect_bus(Bus* b);

  // APU registers are mapped in range $4000-$4013, $4015, $4017
  void cpu_write(uint16_t addr, uint8_t data);
  uint8_t cpu_read(uint16_t addr, bool readonly = false);

  void clk();
  void reset();

  // main output
  float get_audio_sample();

  enum frame_counter_mode {
    FOUR_STEP,
    FIVE_STEP
  };

private:
  Bus* bus = nullptr;

  // https://www.nesdev.org/wiki/APU_Pulse
  struct pulse_channel_T {
    bool enabled = false;

    // $4000 / $4004	DDLC VVVV	Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)
    uint8_t duty_cycle = 0;
    bool length_counter_halt = false;
    bool constant_volume = 0;
    uint8_t volume_envelope = 0;
    
    // $4001 / $4005	EPPP NSSS	Sweep unit: enabled (E), period (P), negate (N), shift (S)
    bool sweep_enabled = false;
    uint8_t sweep_divider_period = 0;
    bool sweep_negate = false;
    uint8_t sweep_shift_count = 0;
    uint8_t sweep_divider = 0;
    bool sweep_reload = false;
    
    // $4002 / $4006	TTTT TTTT	Timer low (T)
    uint16_t timer = 0;
    uint16_t timer_period = 0;
    
    // $4003 / $4007	LLLL LTTT	Length counter load (L), timer high (T)
    uint8_t length_counter = 0;

    uint8_t sequencer_pos = 0;

    uint8_t envelope_divider = 0;
    uint8_t envelope_decay = 0;
    bool envelope_start = false;

    // oputput
    uint8_t sample = 0;

    void clock_timer();
    void clock_envelope();
    void clock_sweep(bool channel1);
    void clock_length_counter();
  } pulse[2];
  
  // duty cycle lut
  static constexpr uint8_t duty_cycles[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0}, // 12.5%
    {0, 1, 1, 0, 0, 0, 0, 0}, // 25%
    {0, 1, 1, 1, 1, 0, 0, 0}, // 50%
    {1, 0, 0, 1, 1, 1, 1, 1}  // 25% negated
  };
    
  struct triangle_channel_T {
    bool enabled = false;
    
    // $4008	CRRR RRRR	Length counter halt / linear counter control (C), linear counter load (R)
    bool ctrl = false;
    uint8_t linear_counter_reload = 0;
    // $4009	---- ----	Unused
    // $400A	TTTT TTTT	Timer low (T)
    uint16_t timer = 0;
    uint16_t timer_period = 0;
    // $400B	LLLL LTTT	Length counter load (L), timer high (T), set linear counter reload flag
    uint8_t length_counter = 0;
    uint8_t linear_counter = 0;
    bool set_linear_counter_reload = false;

    uint8_t sequencer_pos = 0;

    // output
    uint8_t sample = 0;

    void clock_timer();
    void clock_linear_counter();
    void clock_length_counter();
  } triangle;

  static constexpr uint8_t triangle_sequence[32] = {
    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
  };

  
  struct noise_channel_T {
    bool enabled = false;
    
    // $400C	--LC VVVV	Envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)
    bool length_counter_halt = false;
    bool const_volume = false;
    uint8_t volume_envelope = 0;
    // $400D	---- ----	Unused
    // $400E	M--- PPPP	Noise mode (M), noise period (P)
    bool mode = false;
    uint8_t period = 0;
    // $400F	LLLL L---	Length counter load (L)
    uint8_t length_counter = 0;

    uint16_t shift_reg = 1;
    uint16_t timer = 0;

    uint8_t envelope_divider = 0;
    uint8_t envelope_decay = 0;
    bool envelope_start = false;

    // output
    uint8_t sample = 0;

    void clock_timer();
    void clock_envelope();
    void clock_length_counter();
  } noise;

  static constexpr uint16_t noise_period_ntsc[16] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
  };

  struct DMC_channel_T {
    bool enabled = false;

    // $4010	IL-- RRRR	IRQ enable (I), loop (L), frequency (R)
    bool irq_enable = false;
    bool loop = false;
    uint8_t freq = 0;
    // $4011	-DDD DDDD	Load counter (D)
    uint8_t output_level = 0;
    // $4012	AAAA AAAA	Sample address (A)
    uint16_t sample_addr = 0;
    // $4013	LLLL LLLL	Sample length (L)
    uint16_t sample_length = 0;

    // current state;
    uint16_t current_addr = 0;
    uint16_t bytes_left = 0;
    uint8_t sample_buf = 0;
    bool sample_buf_empty = true;

    uint8_t shift_reg = 0;
    uint8_t bits_left = 0;
    bool silence = true;

    uint16_t timer = 0;
    void clock_timer();
  } dmc;

  static constexpr uint16_t dmc_rates_ntsc[16] = {
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
  };
  
  
  static constexpr uint8_t length_counter_table[32] = {
    10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
  };

  // FRAME COUNTER
  uint8_t frame_counter_mode = FOUR_STEP;
  bool irq_inhibit = false;
  bool frame_interrupt = false;
  uint32_t frame_clock_counter = 0;
  bool frame_delay = false;

  void clock_frame_counter();
  // envelope and triangle linear counter
  void clock_quarter_frame();
  // length counters and sweep units
  void clock_half_frame();

  // status register $4015
  void update_status();

  // MIXER
  float pulse_table[31];
  float tnd_table[203];
  void init_mixer_tables();
};

#endif
