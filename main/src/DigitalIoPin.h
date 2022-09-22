class DigitalIoPin {
public:
 DigitalIoPin(int _port, int _pin, bool input = true, bool pullup = true, bool _invert = false);
 DigitalIoPin(const DigitalIoPin &) = delete; //no copy constructor allowed
 DigitalIoPin& operator= (const DigitalIoPin&) = delete;
 virtual ~DigitalIoPin();
 bool read();
 void write(bool value);
private:
 int port;
 int pin;
 bool invert;
};

