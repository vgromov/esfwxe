#ifndef _ese_keyboard_intf_h_
#define _ese_keyboard_intf_h_

class ESE_ABSTRACT EseKeyboardIntf
{
public:
  // Keyboard-Button services
  //
  virtual int buttonsCountGet() const ESE_NOTHROW = 0;
  virtual bool isPressed(int idx) const ESE_NOTHROW = 0;  
  virtual bool isHeld(int idx) const ESE_NOTHROW = 0;
};

#endif // _ese_keyboard_intf_h_
