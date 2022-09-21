class DigitalIoPin {
public:
 DigitalIoPin(int port, int pin, bool input = true, bool pullup = true, bool invert = false);
 DigitalIoPin(const DigitalIoPin &) = delete;
 virtual ~DigitalIoPin();
 bool read();
 void write(bool value);
private:
 // add these as needed
 int port;
 int pin;
 bool invert;
};

