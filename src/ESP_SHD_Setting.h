class ShdSetting {
public:
  ShdSetting * nextSetting;
  const * char name;
  ESP_SmartHomeDevice * parent;

  virtual char * getValueSelectionHtml() = 0;
  virtual char * getLabel() = 0;
  virtual bool setNewValue(const char* _newValue) = 0;
private:
}
