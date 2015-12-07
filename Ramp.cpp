#include "Arduino.h"
#include "Ramp.h"

Ramp::Ramp( boolean rgb )
{
  if (rgb) { _ch_num = 3; } else { _ch_num = 1; } ;
  
  for (byte i = 0; i < _ch_num; i++) {
    _step[i] = 0;
    _sp[i] = 0;
    _value[i] = 0.0;
    _value_hw[i] = 0;
  }

  _at_sp = true;

}

void  Ramp::_calc_steps( byte steps )
{
  _at_sp = true;
  
  for (byte i = 0; i < _ch_num; i++) 
       {
        _step[i] = (_sp[i] - _value[i]) / ((steps==0)?1:(((float) steps)*20) );  
        // time <0, 255> * 20 = <0, 5100>; one step = 12 milisec; longest change 61.2 sec

        if (_step != 0) _at_sp = false;
      }   
}

void Ramp::set_sp( byte color_r, byte color_g, byte color_b, byte steps )
{
  _sp[0] = color_r;
  _sp[1] = color_g;
  _sp[2] = color_b;
  _steps = steps;
  _calc_steps( _steps );
}

void Ramp::set_sp( byte color_w, byte steps )
{
  _sp[0] = color_w;

  _calc_steps( steps );
}

void  Ramp::do_step()
{
   _at_sp = true;
   
   for (byte i = 0; i < _ch_num; i++) 
   {
         if ((_value[i]  > _sp[i] + _step[i]*((_step[i]<0)?-1:1)) || (_value[i]  < _sp[i] - _step[i]*((_step[i]<0)?-1:1) )) 
         {     
          _value[i] = _value[i] + _step[i]; 
          _at_sp = false;
         } else if (_value[i] != _sp[i]) {  
          _value[i] = _sp[i];  
          _at_sp = false;
         }
   }
}

byte  Ramp::get_val( byte index )
{
  return _value[index];
}

boolean Ramp::at_sp()
{
  return _at_sp;
}

byte Ramp::get_sp( byte index )
{
  return _sp[index];
}

byte Ramp::get_steps()
{
  return _steps;
}





