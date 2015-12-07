class Ramp
{
  public:
    Ramp( boolean rgb );
    boolean at_sp();
    void set_sp( byte color_r, byte color_g, byte color_b, byte steps);
    void set_sp( byte color_w, byte change_time);
    void do_step();
    byte get_val( byte index ); 
    byte get_sp( byte index ); 
    byte get_steps();
  private:
    void _calc_steps( byte steps );
    boolean _at_sp;
    byte _ch_num;
    float _step[3];
    float _sp[3];
    float _value[3];
    byte _value_hw[3];
    byte _steps;
};

