# Requirements:

```
Python >= 3.10
Pygame
Pyserial

Arduino board + hardware
```

# Installation:

1. Add Motor_controller library to your arduino libraries.  

2. Make changes to Wire library for arduino:  
   Go to `C:\path\to\library\Wire\src\utility\twi.c @ line 96:`  
   `cbi(TWSR, TWPS0);`  
   CHANGE TO:  
   `sbi(TWSR, TWPS0); \\Changed cbi to sbi`  
3. Upload `HiTechnic\HiTechnic.ino`
4. Run `main.py`
