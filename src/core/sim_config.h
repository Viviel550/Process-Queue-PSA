#pragma once

// Parametry modelu symulacji wspolne dla wszystkich algorytmow.
struct SimConfig {
    // Kara czasowa (w jednostkach symulacji) za kazde przelaczenie kontekstu.
    // 0 = model idealny (przelaczenie nic nie kosztuje).
    int contextSwitchCost = 0;
};
