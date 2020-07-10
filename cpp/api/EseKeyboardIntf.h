#ifndef _ese_keyboard_intf_h_
#define _ese_keyboard_intf_h_

class ESE_ABSTRACT EseKeyboardIntf
{
public:
  // Keyboard-Button services
  //
  virtual void set(int idx, bool pressed, bool held) ESE_NOTHROW = 0;
  virtual int buttonsCountGet() const ESE_NOTHROW = 0;
  virtual bool isFirstEvent() const ESE_NOTHROW = 0;
  virtual bool isPressed(int idx) const ESE_NOTHROW = 0;  
  virtual bool isHeld(int idx) const ESE_NOTHROW = 0;
  virtual bool isPressedPrev(int idx) const ESE_NOTHROW = 0;
  virtual bool isHeldPrev(int idx) const ESE_NOTHROW = 0;
  virtual bool isRepeating(int idx) const ESE_NOTHROW = 0;
  virtual bool stateChanged(int idx) const ESE_NOTHROW = 0;
  virtual bool isPressedAny() const ESE_NOTHROW = 0;
  virtual bool isHeldAny() const ESE_NOTHROW = 0;
};

#endif // _ese_keyboard_intf_h_
