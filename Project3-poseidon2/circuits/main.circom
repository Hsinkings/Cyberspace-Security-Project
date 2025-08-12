    //circuits/main.circom
    pragma circom 2.0.0;
    include "poseidon2_perm.circom";
    include "poseidon2_constants.circom";

    template PoseidonSponge() {
        signal input in0;
        signal input in1;
        signal input pubHash;
        //initial state zeros
        signal state[3];
        state[0] <== 0;
        state[1] <== 0;
        state[2] <== 0;
        state[0] <== state[0] + in0;
        state[1] <== state[1] + in1;
        component P = Poseidon2Perm();
        for (var i = 0; i < 3; i++) P.state_in[i] <== state[i];
        signal out0; out0 <== P.state_out[0];
        pubHash === out0;
    }

    component main = PoseidonSponge();
