#include <gtest/gtest.h>
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

// Simple particle-like struct for testing
struct TestParticle {
    double px, py, pz;
    int pid;
    
    bool operator==(const TestParticle& other) const {
        return px == other.px && py == other.py && pz == other.pz && pid == other.pid;
    }
};

// ============================================================================
// Store/Retrieve Tests (Requirements 4.1, 4.2)
// ============================================================================

TEST(EventStoreTest, StoreAndRetrieveInt) {
    EventStore store;
    store.store("test_int", 42);
    
    auto result = store.retrieve<int>("test_int");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST(EventStoreTest, StoreAndRetrieveDouble) {
    EventStore store;
    store.store("test_double", 3.14159);
    
    auto result = store.retrieve<double>("test_double");
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value(), 3.14159);
}

TEST(EventStoreTest, StoreAndRetrieveString) {
    EventStore store;
    store.store("test_string", std::string("hello world"));
    
    auto result = store.retrieve<std::string>("test_string");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "hello world");
}

TEST(EventStoreTest, StoreAndRetrieveBool) {
    EventStore store;
    store.store("test_bool", true);
    
    auto result = store.retrieve<bool>("test_bool");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST(EventStoreTest, StoreAndRetrieveVector) {
    EventStore store;
    std::vector<int> vec = {1, 2, 3, 4, 5};
    store.store("test_vector", vec);
    
    auto result = store.retrieve<std::vector<int>>("test_vector");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), vec);
}

// ============================================================================
// Type Safety Tests (Requirements 4.2)
// ============================================================================

TEST(EventStoreTest, TypeMismatchReturnsEmpty) {
    EventStore store;
    store.store("test_int", 42);
    
    // Try to retrieve as wrong type
    auto result = store.retrieve<std::string>("test_int");
    EXPECT_FALSE(result.has_value());
}

TEST(EventStoreTest, TypeMismatchDoesNotThrow) {
    EventStore store;
    store.store("test_string", std::string("hello"));
    
    // Should not throw, just return empty optional
    EXPECT_NO_THROW({
        auto result = store.retrieve<int>("test_string");
        EXPECT_FALSE(result.has_value());
    });
}

// ============================================================================
// Missing Key Tests (Requirements 4.3)
// ============================================================================

TEST(EventStoreTest, MissingKeyReturnsEmpty) {
    EventStore store;
    
    auto result = store.retrieve<int>("nonexistent");
    EXPECT_FALSE(result.has_value());
}

TEST(EventStoreTest, ContainsReturnsFalseForMissingKey) {
    EventStore store;
    
    EXPECT_FALSE(store.contains("nonexistent"));
}

TEST(EventStoreTest, ContainsReturnsTrueForExistingKey) {
    EventStore store;
    store.store("existing", 42);
    
    EXPECT_TRUE(store.contains("existing"));
}

// ============================================================================
// Particle Collection Tests (Requirements 4.4)
// ============================================================================

TEST(EventStoreTest, StoreAndRetrieveParticleCollection) {
    EventStore store;
    
    std::vector<TestParticle> particles = {
        {1.0, 2.0, 3.0, 11},
        {4.0, 5.0, 6.0, -11},
        {7.0, 8.0, 9.0, 22}
    };
    
    store.store_particles("electrons", particles);
    
    auto* result = store.get_particles<TestParticle>("electrons");
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->size(), 3);
    EXPECT_EQ((*result)[0], particles[0]);
    EXPECT_EQ((*result)[1], particles[1]);
    EXPECT_EQ((*result)[2], particles[2]);
}

TEST(EventStoreTest, ParticleCollectionMissingKeyReturnsNull) {
    EventStore store;
    
    auto* result = store.get_particles<TestParticle>("nonexistent");
    EXPECT_EQ(result, nullptr);
}

TEST(EventStoreTest, ParticleCollectionTypeMismatchReturnsNull) {
    EventStore store;
    store.store("not_particles", 42);
    
    auto* result = store.get_particles<TestParticle>("not_particles");
    EXPECT_EQ(result, nullptr);
}

TEST(EventStoreTest, EmptyParticleCollection) {
    EventStore store;
    std::vector<TestParticle> empty_particles;
    
    store.store_particles("empty", empty_particles);
    
    auto* result = store.get_particles<TestParticle>("empty");
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->empty());
}

TEST(EventStoreTest, MultipleParticleCollections) {
    EventStore store;
    
    std::vector<TestParticle> electrons = {{1.0, 0.0, 0.0, 11}};
    std::vector<TestParticle> photons = {{0.0, 1.0, 0.0, 22}};
    
    store.store_particles("electrons", electrons);
    store.store_particles("photons", photons);
    
    auto* e_result = store.get_particles<TestParticle>("electrons");
    auto* p_result = store.get_particles<TestParticle>("photons");
    
    ASSERT_NE(e_result, nullptr);
    ASSERT_NE(p_result, nullptr);
    EXPECT_EQ(e_result->size(), 1);
    EXPECT_EQ(p_result->size(), 1);
    EXPECT_EQ((*e_result)[0].pid, 11);
    EXPECT_EQ((*p_result)[0].pid, 22);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(EventStoreTest, OverwriteExistingKey) {
    EventStore store;
    store.store("key", 42);
    store.store("key", 100);
    
    auto result = store.retrieve<int>("key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 100);
}

TEST(EventStoreTest, OverwriteWithDifferentType) {
    EventStore store;
    store.store("key", 42);
    store.store("key", std::string("now a string"));
    
    // Old type should fail
    auto int_result = store.retrieve<int>("key");
    EXPECT_FALSE(int_result.has_value());
    
    // New type should succeed
    auto str_result = store.retrieve<std::string>("key");
    ASSERT_TRUE(str_result.has_value());
    EXPECT_EQ(str_result.value(), "now a string");
}

TEST(EventStoreTest, EmptyStringKey) {
    EventStore store;
    store.store("", 42);
    
    EXPECT_TRUE(store.contains(""));
    auto result = store.retrieve<int>("");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST(EventStoreTest, SpecialCharactersInKey) {
    EventStore store;
    store.store("key/with/slashes", 1);
    store.store("key.with.dots", 2);
    store.store("key:with:colons", 3);
    
    EXPECT_TRUE(store.contains("key/with/slashes"));
    EXPECT_TRUE(store.contains("key.with.dots"));
    EXPECT_TRUE(store.contains("key:with:colons"));
}
