    //circuits/poseidon2_perm.circom
    pragma circom 2.0.0;

    include "poseidon2_constants.circom";

    template Poseidon2Perm() {
        signal input state_in[3];
        signal output state_out[3];

        //copy state
        signal s[3];
        for (var i = 0; i < 3; i++) {
            s[i] <== state_in[i];
        }

        //helpers
        function sbox5(sig x) -> sig y {
            signal x2; signal x4;
            x2 <== x * x;
            x4 <== x2 * x2;
            y <== x4 * x;
            return y;
        }

        //constants
        var RF = PoseidonParams.RF;
        var RP = PoseidonParams.RP;
        var half = RF/2;
        var rcIndex = 0;

        //first half full rounds
        for (var r = 0; r < half; r++) {
            for (var i = 0; i < 3; i++) s[i] <== s[i] + PoseidonParams.ROUND_CONSTANTS[rcIndex + i];
            rcIndex += 3;
            for (var i = 0; i < 3; i++) s[i] <== sbox5(s[i]);
            //MDS multiply
            signal tmp[3];
            for (var i = 0; i < 3; i++) {
                tmp[i] <== 0;
                for (var j = 0; j < 3; j++) tmp[i] <== tmp[i] + PoseidonParams.MDS[i][j] * s[j];
            }
            for (var i = 0; i < 3; i++) s[i] <== tmp[i];
        }

        //partial rounds
        for (var r = 0; r < RP; r++) {
            for (var i = 0; i < 3; i++) s[i] <== s[i] + PoseidonParams.ROUND_CONSTANTS[rcIndex + i];
            rcIndex += 3;
            s[0] <== sbox5(s[0]);
            signal tmp2[3];
            for (var i = 0; i < 3; i++) {
                tmp2[i] <== 0;
                for (var j = 0; j < 3; j++) tmp2[i] <== tmp2[i] + PoseidonParams.MDS[i][j] * s[j];
            }
            for (var i = 0; i < 3; i++) s[i] <== tmp2[i];
        }

        //second half full rounds
        for (var r = 0; r < half; r++) {
            for (var i = 0; i < 3; i++) s[i] <== s[i] + PoseidonParams.ROUND_CONSTANTS[rcIndex + i];
            rcIndex += 3;
            for (var i = 0; i < 3; i++) s[i] <== sbox5(s[i]);
            signal tmp3[3];
            for (var i = 0; i < 3; i++) {
                tmp3[i] <== 0;
                for (var j = 0; j < 3; j++) tmp3[i] <== tmp3[i] + PoseidonParams.MDS[i][j] * s[j];
            }
            for (var i = 0; i < 3; i++) s[i] <== tmp3[i];
        }

        for (var i = 0; i < 3; i++) state_out[i] <== s[i];
    }

    component main = Poseidon2Perm();
