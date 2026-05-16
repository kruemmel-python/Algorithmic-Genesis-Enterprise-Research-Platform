#include "test_harness.hpp"
#include "ag/ag_snn.hpp"

TEST_CASE(snn_runs) {
    ag::SnnConfig cfg;
    cfg.neurons = 32;
    cfg.connections_per_neuron = 4;
    cfg.steps = 20;
    cfg.seed = 7;
    ag::SnnNetwork net(cfg);
    auto r = net.run();
    CHECK_EQ(static_cast<int>(r.spikes_by_step.size()), 20);
    CHECK_TRUE(r.mean_weight >= 0.0);
    CHECK_EQ(static_cast<int>(r.synapse_count), 128);
    CHECK_TRUE(!r.backend_model.empty());
}

TEST_CASE(snn_topology_csr_is_well_formed) {
    ag::SnnConfig cfg;
    cfg.neurons = 16;
    cfg.connections_per_neuron = 3;
    cfg.steps = 1;
    cfg.seed = 9;
    ag::SnnNetwork net(cfg);
    const auto topo = net.make_opencl_topology();

    CHECK_EQ(static_cast<int>(topo.outgoing_offsets.size()), 17);
    CHECK_EQ(static_cast<int>(topo.incoming_offsets.size()), 17);
    CHECK_EQ(static_cast<int>(topo.weights.size()), 48);
    CHECK_EQ(static_cast<int>(topo.outgoing_indices.size()), 48);
    CHECK_EQ(static_cast<int>(topo.incoming_sources.size()), 48);
    CHECK_EQ(static_cast<int>(topo.incoming_edge_ids.size()), 48);
    CHECK_EQ(static_cast<int>(topo.outgoing_offsets.front()), 0);
    CHECK_EQ(static_cast<int>(topo.outgoing_offsets.back()), 48);
    CHECK_EQ(static_cast<int>(topo.incoming_offsets.front()), 0);
    CHECK_EQ(static_cast<int>(topo.incoming_offsets.back()), 48);
}
