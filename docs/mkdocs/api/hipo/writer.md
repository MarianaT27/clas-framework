# HIPO Writer

Writing HIPO files.

## hipo::writer

```cpp
namespace hipo {

class writer {
public:
    writer();
    
    auto open(const std::string& filename) -> void;
    auto close() -> void;
    
    auto set_dictionary(dictionary& dict) -> void;
    auto add_event(event& evt) -> void;
    
    auto flush() -> void;
};

}
```

## Usage

```cpp
hipo::writer writer;
hipo::dictionary dict;

// Define banks
dict.add_schema("MY::Data", "pid/I,px/F,py/F,pz/F");

writer.set_dictionary(dict);
writer.open("output.hipo");

// Create and fill events
hipo::event event;
hipo::bank my_data(dict.get_schema("MY::Data"), 10);

for (int i = 0; i < 10; i++) {
    my_data.put_int("pid", i, 11);
    my_data.put_float("px", i, 0.5);
    my_data.put_float("py", i, 0.3);
    my_data.put_float("pz", i, 2.0);
}

event.add_bank(my_data);
writer.add_event(event);

writer.close();
```

## Schema Types

| Type | Code | Description |
|------|------|-------------|
| `B` | byte | 8-bit integer |
| `S` | short | 16-bit integer |
| `I` | int | 32-bit integer |
| `L` | long | 64-bit integer |
| `F` | float | 32-bit float |
| `D` | double | 64-bit float |
