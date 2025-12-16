# HIPO Reader

Reading HIPO files.

## hipo::reader

```cpp
namespace hipo {

class reader {
public:
    reader();
    
    auto open(const std::string& filename) -> void;
    auto next() -> bool;
    auto read(event& evt) -> void;
    
    auto get_dictionary() -> dictionary&;
};

}
```

## hipo::chain

For processing multiple files with parallelization:

```cpp
namespace hipo {

class chain {
public:
    chain(int num_threads = 1, bool progress = true, bool verbose = false);
    
    auto add(const std::string& filename) -> void;
    
    template<typename Func>
    auto process(Func&& callback, double percentage = 100.0) -> void;
};

}
```

## Usage

### Single File

```cpp
hipo::reader reader;
reader.open("data.hipo");

hipo::event event;
while (reader.next()) {
    reader.read(event);
    // Process event
}
```

### Multiple Files (Parallel)

```cpp
hipo::chain chain(4, true, false);  // 4 threads, progress bar, not verbose
chain.add("data1.hipo");
chain.add("data2.hipo");

chain.process([](hipo::iter_event& event, int file_idx, long evt_idx) {
    auto rec_particle = event.get_bank_map("REC::Particle");
    // Process event
}, 100.0);  // Process 100% of events
```

## Framework Integration

The `Application` class handles HIPO reading internally:

```cpp
Framework::Application app("MyAnalysis");
app.set_input_files({"data1.hipo", "data2.hipo"});
app.set_num_threads(4);
// HIPO reading is handled automatically
```
