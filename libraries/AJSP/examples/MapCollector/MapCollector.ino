#include <MapCollector.hpp>

void setup() {
  Serial.begin(57600);
}

bool predicate(const std::string& path, const std::string& value)
{
  String p(path.c_str());
  return p.startsWith("/root/0/");    //select only elements with path beginning with /root/0
}

const char data[] = "[{'a': 'A', 'b': 'B'}, 'test']";

void loop() {
  MapCollector mc(predicate);
  for (const auto& c: data)
  {
    mc.parse(c);
  }

  for (const auto& p: mc.getValues())
  {
    Serial.printf("%s - %s\n", p.first.c_str(), p.second.c_str());
  }
  // Prints:
  // /root/0/a - A
  // /root/0/b - B
}
