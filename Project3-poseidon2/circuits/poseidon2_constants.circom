    //circuits/poseidon2_constants.circom
    pragma circom 2.0.0;

    template PoseidonParams() {
        signal const T = 3;
        signal const D = 5;
        signal const RF = 2; //very small for demo
        signal const RP = 1;
        signal const MDS[3][3];
        MDS[0][0] <-- 1; MDS[0][1] <-- 0; MDS[0][2] <-- 0;
        MDS[1][0] <-- 0; MDS[1][1] <-- 1; MDS[1][2] <-- 0;
        MDS[2][0] <-- 0; MDS[2][1] <-- 0; MDS[2][2] <-- 1;
        signal const ROUND_CONSTANTS[9];
        ROUND_CONSTANTS[0] <-- 0; ROUND_CONSTANTS[1] <-- 0; ROUND_CONSTANTS[2] <-- 0;
        ROUND_CONSTANTS[3] <-- 0; ROUND_CONSTANTS[4] <-- 0; ROUND_CONSTANTS[5] <-- 0;
        ROUND_CONSTANTS[6] <-- 0; ROUND_CONSTANTS[7] <-- 0; ROUND_CONSTANTS[8] <-- 0;
    }
    component PoseidonParams = PoseidonParams();
