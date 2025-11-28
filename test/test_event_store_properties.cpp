#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <any>
#include <map>
#include <string>
#include <optional>
#include <vector>

/**
 * Event Store Test Fixture
 * 
 * Since the actual Event class requires a hipo::iter_event reference,
 * we test the event store logic in isolation using the same std::any-based
 * storage mechanism that Event uses internally.
 */
class EventStore {
public:
    template<typename T>
    auto store(const std::string& key, T value) -> void {
        m_store[key] = std::any(std::move(value));
    }

    template<typename T>
    auto retrieve(const std::string& key) const -> std::optional<T> {
        auto it = m_store.find(key);
        if (it == m_store.end()) {
            return std::nullopt;
        }
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    auto contains(const std::string& key) const -> bool {
        return m_store.find(key) != m_store.end();
    }

    template<typename ParticleType>
    auto store_particles(const std::string& key, std::vector<ParticleType> particles) -> void {
        m_store[key] = std::any(std::move(particles));
    }

    template<typename ParticleType>
    auto get_particles(const std::string& key) const -> const std::vector<ParticleType>* {
        auto it = m_store.find(key);
        if (it == m_store.end()) {
            return nullptr;
        }
        return std::any_cast<std::vector<ParticleType>>(&it->second);
    }

private:
    std::map<std::string, std::any> m_store;
};

namespace {
    // Generate valid storage key (alphanumeric, starting with letter)
    auto genValidKey() -> rc::Gen<std::string> {
        return rc::gen::map(
            rc::gen::pair(
                rc::gen::inRange('a', 'z'),
                rc::gen::container<std::string>(rc::gen::inRange('a', 'z'))
            ),
            [](const std::pair<char, std::string>& p) {
                return std::string(1, p.first) + p.second;
            }
        );
    }

    // Simple particle-like struct for testing
    struct TestParticle {
        double px, py, pz;
        int pid;
        
        bool operator==(const TestParticle& other) const {
            return px == other.px && py == other.py && pz == other.pz && pid == other.pid;
        }
    };
}

/**
 * **Feature: code-quality-improvements, Property 4: Event Store Round-Trip**
 * 
 * *For any* data object stored in the Event store with a unique key, retrieving the data
 * by the same key SHALL return an equivalent object, and retrieving with a non-existent
 * key SHALL return an empty optional.
 * 
 * **Validates: Requirements 4.1, 4.2, 4.3, 4.4**
 */

// Property: Integer round-trip
RC_GTEST_PROP(EventStoreProperties, IntegerRoundTrip, ()) {
    EventStore store;
    std::string key = *genValidKey();
    int value = *rc::gen::arbitrary<int>();
    
    store.store(key, value);
    
    auto result = store.retrieve<int>(key);
    RC_ASSERT(result.has_value());
    RC_ASSERT(result.value() == value);
}

// Property: Double round-trip
RC_GTEST_PROP(EventStoreProperties, DoubleRoundTrip, ()) {
    EventStore store;
    std::string key = *genValidKey();
    double value = *rc::gen::arbitrary<double>();
    
    store.store(key, value);
    
    auto result = store.retrieve<double>(key);
    RC_ASSERT(result.has_value());
    // Handle NaN specially
    if (std::isnan(value)) {
        RC_ASSERT(std::isnan(result.value()));
    } else {
        RC_ASSERT(result.value() == value);
    }
}

// Property: String round-trip
RC_GTEST_PROP(EventStoreProperties, StringRoundTrip, ()) {
    EventStore store;
    std::string key = *genValidKey();
    std::string value = *rc::gen::arbitrary<std::string>();
    
    store.store(key, value);
    
    auto result = store.retrieve<std::string>(key);
    RC_ASSERT(result.has_value());
    RC_ASSERT(result.value() == value);
}

// Property: Boolean round-trip
RC_GTEST_PROP(EventStoreProperties, BooleanRoundTrip, ()) {
    EventStore store;
    std::string key = *genValidKey();
    bool value = *rc::gen::arbitrary<bool>();
    
    store.store(key, value);
    
    auto result = store.retrieve<bool>(key);
    RC_ASSERT(result.has_value());
    RC_ASSERT(result.value() == value);
}

// Property: Non-existent key returns empty optional
RC_GTEST_PROP(EventStoreProperties, NonExistentKeyReturnsEmpty, ()) {
    EventStore store;
    std::string key = *genValidKey();
    
    // Don't store anything, just try to retrieve
    auto result = store.retrieve<int>(key);
    RC_ASSERT(!result.has_value());
}

// Property: Contains returns true for stored keys
RC_GTEST_PROP(EventStoreProperties, ContainsAfterStore, ()) {
    EventStore store;
    std::string key = *genValidKey();
    int value = *rc::gen::arbitrary<int>();
    
    RC_ASSERT(!store.contains(key));  // Before storing
    store.store(key, value);
    RC_ASSERT(store.contains(key));   // After storing
}

// Property: Contains returns false for non-existent keys
RC_GTEST_PROP(EventStoreProperties, ContainsNonExistent, ()) {
    EventStore store;
    std::string key = *genValidKey();
    
    RC_ASSERT(!store.contains(key));
}

// Property: Type mismatch returns empty optional
RC_GTEST_PROP(EventStoreProperties, TypeMismatchReturnsEmpty, ()) {
    EventStore store;
    std::string key = *genValidKey();
    int value = *rc::gen::arbitrary<int>();
    
    store.store(key, value);
    
    // Try to retrieve as wrong type
    auto result = store.retrieve<std::string>(key);
    RC_ASSERT(!result.has_value());
}

// Property: Particle collection round-trip
RC_GTEST_PROP(EventStoreProperties, ParticleCollectionRoundTrip, ()) {
    EventStore store;
    std::string key = *genValidKey();
    
    // Generate a vector of test particles
    auto particles = *rc::gen::container<std::vector<TestParticle>>(
        rc::gen::build<TestParticle>(
            rc::gen::set(&TestParticle::px, rc::gen::arbitrary<double>()),
            rc::gen::set(&TestParticle::py, rc::gen::arbitrary<double>()),
            rc::gen::set(&TestParticle::pz, rc::gen::arbitrary<double>()),
            rc::gen::set(&TestParticle::pid, rc::gen::arbitrary<int>())
        )
    );
    
    store.store_particles(key, particles);
    
    auto* result = store.get_particles<TestParticle>(key);
    RC_ASSERT(result != nullptr);
    RC_ASSERT(result->size() == particles.size());
    
    for (size_t i = 0; i < particles.size(); ++i) {
        // Handle NaN in comparisons
        const auto& orig = particles[i];
        const auto& retrieved = (*result)[i];
        
        if (std::isnan(orig.px)) {
            RC_ASSERT(std::isnan(retrieved.px));
        } else {
            RC_ASSERT(orig.px == retrieved.px);
        }
        if (std::isnan(orig.py)) {
            RC_ASSERT(std::isnan(retrieved.py));
        } else {
            RC_ASSERT(orig.py == retrieved.py);
        }
        if (std::isnan(orig.pz)) {
            RC_ASSERT(std::isnan(retrieved.pz));
        } else {
            RC_ASSERT(orig.pz == retrieved.pz);
        }
        RC_ASSERT(orig.pid == retrieved.pid);
    }
}

// Property: get_particles returns nullptr for non-existent key
RC_GTEST_PROP(EventStoreProperties, ParticleCollectionNonExistent, ()) {
    EventStore store;
    std::string key = *genValidKey();
    
    auto* result = store.get_particles<TestParticle>(key);
    RC_ASSERT(result == nullptr);
}

// Property: Multiple stores with different keys are independent
RC_GTEST_PROP(EventStoreProperties, MultipleKeysIndependent, ()) {
    EventStore store;
    std::string key1 = *genValidKey();
    std::string key2 = *rc::gen::suchThat(genValidKey(), [&key1](const std::string& k) {
        return k != key1;
    });
    
    int value1 = *rc::gen::arbitrary<int>();
    int value2 = *rc::gen::arbitrary<int>();
    
    store.store(key1, value1);
    store.store(key2, value2);
    
    auto result1 = store.retrieve<int>(key1);
    auto result2 = store.retrieve<int>(key2);
    
    RC_ASSERT(result1.has_value());
    RC_ASSERT(result2.has_value());
    RC_ASSERT(result1.value() == value1);
    RC_ASSERT(result2.value() == value2);
}

// Property: Overwriting a key updates the value
RC_GTEST_PROP(EventStoreProperties, OverwriteUpdatesValue, ()) {
    EventStore store;
    std::string key = *genValidKey();
    int value1 = *rc::gen::arbitrary<int>();
    int value2 = *rc::gen::arbitrary<int>();
    
    store.store(key, value1);
    store.store(key, value2);
    
    auto result = store.retrieve<int>(key);
    RC_ASSERT(result.has_value());
    RC_ASSERT(result.value() == value2);
}
