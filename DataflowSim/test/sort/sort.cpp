#include "./sort.h"

using namespace DFSimTest;

Dfg Sort_Test::dfg;

string Sort_Test::filePath = "./test/sort/input.dat";

uint Sort_Test::SIZE = pow(2, 13);

vector<int> Sort_Test::temp(3 * SIZE);
vector<int> Sort_Test::a;

uint Sort_Test::temp_BaseAddr = 0;
uint Sort_Test::a_BaseAddr = 5159;

void Sort_Test::genInputData()
{
    std::ofstream _genData_file;
    _genData_file.open(filePath);

    for (size_t i = 0; i < temp.size(); ++i)
    {
        int a = rand() % SIZE + 1;
        _genData_file << a << std::endl;
    }
}

void Sort_Test::generateData()
{
    std::ifstream _infile;
    _infile.open(filePath);

    uint k = 0;
    string temp_str;
    while (getline(_infile, temp_str))
    {
        temp[k] = std::stoi(temp_str);
        k++;
    }
}

void Sort_Test::generateDfg()
{
    //** ControlRegion
    dfg.controlTree.addControlRegion(
        { make_tuple<string, string, string>("loop_m", "Loop", "Null"),
         make_tuple<string, string, string>("loop_i", "Loop", "Null"),
         make_tuple<string, string, string>("branch_to_true", "Branch", "truePath"),
         make_tuple<string, string, string>("loop_k1", "Loop", "Null"),
         make_tuple<string, string, string>("branch_tmp1_true", "Branch", "truePath"),
         make_tuple<string, string, string>("branch_tmp1_false", "Branch", "falsePath"),
         make_tuple<string, string, string>("branch_to_false", "Branch", "falsePath"),
         make_tuple<string, string, string>("loop_k2", "Loop", "Null"),
         make_tuple<string, string, string>("branch_tmp2_true", "Branch", "truePath"),
         make_tuple<string, string, string>("branch_tmp2_false", "Branch", "falsePath")
        });

    dfg.controlTree.addLowerControlRegion("loop_m", { "loop_i" });
    dfg.controlTree.addLowerControlRegion("loop_i", { "branch_to_true", "branch_to_false" });
    dfg.controlTree.addLowerControlRegion("branch_to_true", { "loop_k1" });
    dfg.controlTree.addLowerControlRegion("loop_k1", { "branch_tmp1_true", "branch_tmp1_false" });
    dfg.controlTree.addLowerControlRegion("branch_to_false", { "loop_k2" });
    dfg.controlTree.addLowerControlRegion("loop_k2", { "branch_tmp2_true", "branch_tmp2_false" });

    dfg.controlTree.completeControlRegionHierarchy();

    //** Node
    // global_const
    // loop_m
    dfg.addNode("begin", "Nop");
    dfg.addNode("end", "Nop", {}, { "m" });
    dfg.addNode("m", "Loop_head", {}, { "begin" });
    dfg.addNode("m_lc", "Nop", { "m" });

    // loop_i
    dfg.addNode("i", "Loop_head", {}, { "m_lc" });
    //dfg.addNode("i_lc", "Nop", { "i" });
    dfg.addNode("i_k1", "Nop", { "i" });  // To loop k1
    dfg.addNode("i_k2", "Nop", { "i" });  // To loop k2
    dfg.addNode("m_m", "Add", { "m_lc" });
    //dfg.addNode("mid", "Sub", { "i_m" });
    dfg.addNode("i_m_m", "Add", { "i", "m_m" });
    dfg.addNode("to", "Sub", { "i_m_m" });
    dfg.addNode("to_k1", "Nop", { "to_cmp", "to" });
    dfg.addNode("to_k2", "Nop", { "to_cmp", "to" });
    dfg.addNode("to_cmp", "Cmp", { "to"});  // if (to < SIZE)
    //dfg.addNode("to_cmp_k1", "Nop", { "to_cmp" });
    //dfg.addNode("to_cmp_k2", "Nop", { "to_cmp" });
    dfg.addNode("br_merge_tmp1_shadow", "Nop", { "br_merge_tmp1" });
    dfg.addNode("br_merge_tmp2_shadow", "Nop", { "br_merge_tmp2" });
    dfg.addNode("br_merge_to", "selPartial", { "to_cmp", "br_merge_tmp1_shadow", "br_merge_tmp2_shadow" });

    // True path
    dfg.addNode("k1", "Loop_head", {}, { "to_k1" });
    dfg.addNode("tmp_j1", "Load", { "to_k1" }, &temp, temp_BaseAddr);
    dfg.addNode("tmp_i1", "Load", { "i_k1" }, &temp, temp_BaseAddr);
    dfg.addNode("tmp1_cmp", "Cmp", { "tmp_j1", "tmp_i1" });
    dfg.addNode("k1_cond", "Nop", { "tmp1_cmp", "k1" });
    dfg.addNode("br_merge_tmp1", "selPartial", { "tmp1_cmp", "to1_update", "i1_update" });

    // if
    dfg.addNode("a_update_k1_true", "Store", { "k1_cond", "tmp_j1" }, &a, a_BaseAddr);
    dfg.addNode("to1_update", "Sub", { "tmp1_cmp", "to_k1" });
    //else
    dfg.addNode("a_update_k1_false", "Store", { "k1_cond", "tmp_j1" }, &a, a_BaseAddr);
    dfg.addNode("i1_update", "Add", { "tmp1_cmp", "i_k1" });

    // False path
    dfg.addNode("k2", "Loop_head", {}, { "to_k2" });
    dfg.addNode("tmp_j2", "Load", { "to_k2" }, &temp, temp_BaseAddr);
    dfg.addNode("tmp_i2", "Load", { "i_k2" }, &temp, temp_BaseAddr);
    dfg.addNode("tmp2_cmp", "Cmp", { "tmp_j2", "tmp_i2" });
    dfg.addNode("k2_cond", "Nop", { "tmp2_cmp", "k2" });
    dfg.addNode("br_merge_tmp2", "selPartial", { "tmp2_cmp", "to2_update", "i2_update" });

    // if
    dfg.addNode("a_update_k2_true", "Store", { "k2_cond", "tmp_j2" }, &a, a_BaseAddr);
    dfg.addNode("to2_update", "Sub", { "tmp2_cmp", "to_k2" });
    //else
    dfg.addNode("a_update_k2_false", "Store", { "k2_cond", "tmp_j2" }, &a, a_BaseAddr);
    dfg.addNode("i2_update", "Add", { "tmp2_cmp", "i_k2" });

    dfg.completeConnect();
    dfg.removeRedundantConnect();

    //** Add nodes to controlTree
    dfg.addNodes2CtrlTree("loop_m", { "begin", "end", "m", "m_lc" });
    dfg.addNodes2CtrlTree("loop_i", { "i", /*"i_lc",*/ /*"i_k1", "i_k2",*/ "m_m", "i_m_m", "to",/*"to_k1","to_k2",*/ "to_cmp",/*"to_cmp_k1","to_cmp_k2",*/ "br_merge_to" });
    dfg.addNodes2CtrlTree("branch_to_true", { "to_k1", "i_k1", "br_merge_tmp1_shadow" });
    dfg.addNodes2CtrlTree("loop_k1", { "k1", "tmp_j1", "tmp_i1", "tmp1_cmp", "k1_cond", "br_merge_tmp1" });
    dfg.addNodes2CtrlTree("branch_tmp1_true", { "a_update_k1_true", "to1_update" });
    dfg.addNodes2CtrlTree("branch_tmp1_false", { "a_update_k1_false", "i1_update" });
    dfg.addNodes2CtrlTree("branch_to_false", { "to_k2", "i_k2", "br_merge_tmp2_shadow" });
    dfg.addNodes2CtrlTree("loop_k2", { "k2", "tmp_j2", "tmp_i2", "tmp2_cmp", "k2_cond", "br_merge_tmp2" });
    dfg.addNodes2CtrlTree("branch_tmp2_true", { "a_update_k2_true", "to2_update" });
    dfg.addNodes2CtrlTree("branch_tmp2_false", { "a_update_k2_false", "i2_update" });
   

    //** Indicate the tail node for each loop region
    dfg.setTheTailNode("loop_m", "i");
    dfg.setTheTailNode("loop_i", "br_merge_to");
    dfg.setTheTailNode("loop_k1", "br_merge_tmp1");
    dfg.setTheTailNode("loop_k2", "br_merge_tmp2");

    dfg.plotDot();
}


// Graph partition
void Sort_Test::graphPartition(ChanGraph& chanGraph, int partitionNum)
{
    switch (partitionNum)
    {
    case 1:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_m")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_i_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_to")->subgraphId = 0;
        chanGraph.getNode("Chan_to_cmp")->subgraphId = 0;
        chanGraph.getNode("Chan_i_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_k2")->subgraphId = 0;
        chanGraph.getNode("Chan_to_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_to_k2")->subgraphId = 0;
        //chanGraph.getNode("Chan_to_cmp_k1")->subgraphId = 0;
        //chanGraph.getNode("Chan_to_cmp_k2")->subgraphId = 0;
        //** K1
        chanGraph.getNode("Lc_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_k1_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_to_k1_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Lse_tmp_i1")->subgraphId = 0;
        chanGraph.getNode("Lse_tmp_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_tmp1_cmp")->subgraphId = 0;
        chanGraph.getNode("Chan_k1_cond")->subgraphId = 0;
        // True
        chanGraph.getNode("Chan_to1_update")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_k1_true")->subgraphId = 0;
        // False
        chanGraph.getNode("Chan_i1_update")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_k1_false")->subgraphId = 0;

        chanGraph.getNode("Chan_br_merge_tmp1")->subgraphId = 0;
        //**
        chanGraph.getNode("Chan_br_merge_tmp1_shadow")->subgraphId = 0;

        //** K2
        chanGraph.getNode("Lc_k2")->subgraphId = 0;
        chanGraph.getNode("Chan_i_k2_scatter_loop_k2")->subgraphId = 0;
        chanGraph.getNode("Chan_to_k2_scatter_loop_k2")->subgraphId = 0;
        chanGraph.getNode("Lse_tmp_i2")->subgraphId = 0;
        chanGraph.getNode("Lse_tmp_j2")->subgraphId = 0;
        chanGraph.getNode("Chan_tmp2_cmp")->subgraphId = 0;
        chanGraph.getNode("Chan_k2_cond")->subgraphId = 0;

        chanGraph.getNode("Chan_to2_update")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_k2_true")->subgraphId = 0;

        chanGraph.getNode("Chan_i2_update")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_k2_false")->subgraphId = 0;

        chanGraph.getNode("Chan_br_merge_tmp2")->subgraphId = 0;
        //**
        chanGraph.getNode("Chan_br_merge_tmp2_shadow")->subgraphId = 0;
        chanGraph.getNode("Chan_br_merge_to")->subgraphId = 0;

        break;
    }
    case 2:
    {
        //chanGraph.getNode("Chan_begin")->subgraphId = 0;
        //chanGraph.getNode("Lc_m")->subgraphId = 0;
        //chanGraph.getNode("Chan_m_lc")->subgraphId = 0;
        //chanGraph.getNode("Chan_end")->subgraphId = 0;

        //chanGraph.getNode("Lc_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_m_lc_scatter_loop_i")->subgraphId = 0;
        //chanGraph.getNode("Chan_m_m")->subgraphId = 0;
        //chanGraph.getNode("Chan_i_m_m")->subgraphId = 0;
        //chanGraph.getNode("Chan_to")->subgraphId = 0;
        //chanGraph.getNode("Chan_to_cmp")->subgraphId = 0;

        //chanGraph.getNode("Chan_to_k1")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_k1")->subgraphId = 1;
        ////** K1
        //chanGraph.getNode("Lc_k1")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_k1_scatter_loop_k1")->subgraphId = 1;
        //chanGraph.getNode("Chan_to_k1_scatter_loop_k1")->subgraphId = 1;
        //chanGraph.getNode("Lse_tmp_i1")->subgraphId = 1;
        //chanGraph.getNode("Lse_tmp_j1")->subgraphId = 1;
        //chanGraph.getNode("Chan_tmp1_cmp")->subgraphId = 1;
        //chanGraph.getNode("Chan_k1_cond")->subgraphId = 1;
        //// True
        //chanGraph.getNode("Chan_to1_update")->subgraphId = 1;
        //chanGraph.getNode("Lse_a_update_k1_true")->subgraphId = 1;
        //// False
        //chanGraph.getNode("Chan_i1_update")->subgraphId = 1;
        //chanGraph.getNode("Lse_a_update_k1_false")->subgraphId = 1;

        //chanGraph.getNode("Chan_br_merge_tmp1")->subgraphId = 1;
        ////**
        //chanGraph.getNode("Chan_br_merge_tmp1_shadow")->subgraphId = 1;

        //chanGraph.getNode("Chan_to_k2")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_k2")->subgraphId = 1;
        ////** K2
        //chanGraph.getNode("Lc_k2")->subgraphId = 1;
        //chanGraph.getNode("Chan_i_k2_scatter_loop_k2")->subgraphId = 1;
        //chanGraph.getNode("Chan_to_k2_scatter_loop_k2")->subgraphId = 1;
        //chanGraph.getNode("Lse_tmp_i2")->subgraphId = 1;
        //chanGraph.getNode("Lse_tmp_j2")->subgraphId = 1;
        //chanGraph.getNode("Chan_tmp2_cmp")->subgraphId = 1;
        //chanGraph.getNode("Chan_k2_cond")->subgraphId = 1;

        //chanGraph.getNode("Chan_to2_update")->subgraphId = 1;
        //chanGraph.getNode("Lse_a_update_k2_true")->subgraphId = 1;

        //chanGraph.getNode("Chan_i2_update")->subgraphId = 1;
        //chanGraph.getNode("Lse_a_update_k2_false")->subgraphId = 1;

        //chanGraph.getNode("Chan_br_merge_tmp2")->subgraphId = 1;
        ////**
        //chanGraph.getNode("Chan_br_merge_tmp2_shadow")->subgraphId = 1;
        //chanGraph.getNode("Chan_br_merge_to")->subgraphId = 1;

        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_m")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_i_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_to")->subgraphId = 0;
        chanGraph.getNode("Chan_to_cmp")->subgraphId = 0;

        chanGraph.getNode("Chan_to_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_k1")->subgraphId = 0;
        //** K1
        chanGraph.getNode("Lc_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_i_k1_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Chan_to_k1_scatter_loop_k1")->subgraphId = 0;
        chanGraph.getNode("Lse_tmp_i1")->subgraphId = 0;
        chanGraph.getNode("Lse_tmp_j1")->subgraphId = 0;
        chanGraph.getNode("Chan_tmp1_cmp")->subgraphId = 0;
        chanGraph.getNode("Chan_k1_cond")->subgraphId = 0;
        // True
        chanGraph.getNode("Chan_to1_update")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_k1_true")->subgraphId = 0;
        // False
        chanGraph.getNode("Chan_i1_update")->subgraphId = 0;
        chanGraph.getNode("Lse_a_update_k1_false")->subgraphId = 0;

        chanGraph.getNode("Chan_br_merge_tmp1")->subgraphId = 0;
        //**
        chanGraph.getNode("Chan_br_merge_tmp1_shadow")->subgraphId = 0;

        chanGraph.getNode("Chan_to_k2")->subgraphId = 1;
        chanGraph.getNode("Chan_i_k2")->subgraphId = 1;
        //** K2
        chanGraph.getNode("Lc_k2")->subgraphId = 1;
        chanGraph.getNode("Chan_i_k2_scatter_loop_k2")->subgraphId = 1;
        chanGraph.getNode("Chan_to_k2_scatter_loop_k2")->subgraphId = 1;
        chanGraph.getNode("Lse_tmp_i2")->subgraphId = 1;
        chanGraph.getNode("Lse_tmp_j2")->subgraphId = 1;
        chanGraph.getNode("Chan_tmp2_cmp")->subgraphId = 1;
        chanGraph.getNode("Chan_k2_cond")->subgraphId = 1;

        chanGraph.getNode("Chan_to2_update")->subgraphId = 1;
        chanGraph.getNode("Lse_a_update_k2_true")->subgraphId = 1;

        chanGraph.getNode("Chan_i2_update")->subgraphId = 1;
        chanGraph.getNode("Lse_a_update_k2_false")->subgraphId = 1;

        chanGraph.getNode("Chan_br_merge_tmp2")->subgraphId = 1;
        //**
        chanGraph.getNode("Chan_br_merge_tmp2_shadow")->subgraphId = 1;
        chanGraph.getNode("Chan_br_merge_to")->subgraphId = 1;

        break;
    }
    case 3:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_m")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_i_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_to")->subgraphId = 0;
        chanGraph.getNode("Chan_to_cmp")->subgraphId = 0;

        chanGraph.getNode("Chan_to_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_k1")->subgraphId = 1;
        //** K1
        chanGraph.getNode("Lc_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_k1_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_to_k1_scatter_loop_k1")->subgraphId = 1;
        chanGraph.getNode("Lse_tmp_i1")->subgraphId = 1;
        chanGraph.getNode("Lse_tmp_j1")->subgraphId = 1;
        chanGraph.getNode("Chan_tmp1_cmp")->subgraphId = 1;
        chanGraph.getNode("Chan_k1_cond")->subgraphId = 1;
        // True
        chanGraph.getNode("Chan_to1_update")->subgraphId = 1;
        chanGraph.getNode("Lse_a_update_k1_true")->subgraphId = 1;
        // False
        chanGraph.getNode("Chan_i1_update")->subgraphId = 1;
        chanGraph.getNode("Lse_a_update_k1_false")->subgraphId = 1;

        chanGraph.getNode("Chan_br_merge_tmp1")->subgraphId = 1;
        //**
        chanGraph.getNode("Chan_br_merge_tmp1_shadow")->subgraphId = 2;

        chanGraph.getNode("Chan_to_k2")->subgraphId = 2;
        chanGraph.getNode("Chan_i_k2")->subgraphId = 2;
        //** K2
        chanGraph.getNode("Lc_k2")->subgraphId = 2;
        chanGraph.getNode("Chan_i_k2_scatter_loop_k2")->subgraphId = 2;
        chanGraph.getNode("Chan_to_k2_scatter_loop_k2")->subgraphId = 2;
        chanGraph.getNode("Lse_tmp_i2")->subgraphId = 2;
        chanGraph.getNode("Lse_tmp_j2")->subgraphId = 2;
        chanGraph.getNode("Chan_tmp2_cmp")->subgraphId = 2;
        chanGraph.getNode("Chan_k2_cond")->subgraphId = 2;

        chanGraph.getNode("Chan_to2_update")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_k2_true")->subgraphId = 2;

        chanGraph.getNode("Chan_i2_update")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_k2_false")->subgraphId = 2;

        chanGraph.getNode("Chan_br_merge_tmp2")->subgraphId = 2;
        //**
        chanGraph.getNode("Chan_br_merge_tmp2_shadow")->subgraphId = 2;
        chanGraph.getNode("Chan_br_merge_to")->subgraphId = 2;

        break;
    }
    case 4:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_m")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_i_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_to")->subgraphId = 0;
        chanGraph.getNode("Chan_to_cmp")->subgraphId = 0;

        chanGraph.getNode("Chan_to_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_k1")->subgraphId = 1;
        //** K1
        chanGraph.getNode("Lc_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_k1_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_to_k1_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Lse_tmp_i1")->subgraphId = 2;
        chanGraph.getNode("Lse_tmp_j1")->subgraphId = 2;
        chanGraph.getNode("Chan_tmp1_cmp")->subgraphId = 2;
        chanGraph.getNode("Chan_k1_cond")->subgraphId = 2;
        // True
        chanGraph.getNode("Chan_to1_update")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_k1_true")->subgraphId = 2;
        // False
        chanGraph.getNode("Chan_i1_update")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_k1_false")->subgraphId = 2;

        chanGraph.getNode("Chan_br_merge_tmp1")->subgraphId = 2;
        //**

        chanGraph.getNode("Chan_br_merge_tmp1_shadow")->subgraphId = 3;

        chanGraph.getNode("Chan_to_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_i_k2")->subgraphId = 3;
        //** K2
        chanGraph.getNode("Lc_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_i_k2_scatter_loop_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_to_k2_scatter_loop_k2")->subgraphId = 3;
        chanGraph.getNode("Lse_tmp_i2")->subgraphId = 3;
        chanGraph.getNode("Lse_tmp_j2")->subgraphId = 3;
        chanGraph.getNode("Chan_tmp2_cmp")->subgraphId = 3;
        chanGraph.getNode("Chan_k2_cond")->subgraphId = 3;

        chanGraph.getNode("Chan_to2_update")->subgraphId = 3;
        chanGraph.getNode("Lse_a_update_k2_true")->subgraphId = 3;

        chanGraph.getNode("Chan_i2_update")->subgraphId = 3;
        chanGraph.getNode("Lse_a_update_k2_false")->subgraphId = 3;

        chanGraph.getNode("Chan_br_merge_tmp2")->subgraphId = 3;
        //**
        chanGraph.getNode("Chan_br_merge_tmp2_shadow")->subgraphId = 3;
        chanGraph.getNode("Chan_br_merge_to")->subgraphId = 3;

        break;
    }
    case 5:
    {
        chanGraph.getNode("Chan_begin")->subgraphId = 0;
        chanGraph.getNode("Lc_m")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc")->subgraphId = 0;
        chanGraph.getNode("Chan_end")->subgraphId = 0;

        chanGraph.getNode("Lc_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_lc_scatter_loop_i")->subgraphId = 0;
        chanGraph.getNode("Chan_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_i_m_m")->subgraphId = 0;
        chanGraph.getNode("Chan_to")->subgraphId = 0;
        chanGraph.getNode("Chan_to_cmp")->subgraphId = 0;

        chanGraph.getNode("Chan_to_k1")->subgraphId = 1;
        chanGraph.getNode("Chan_i_k1")->subgraphId = 1;
        //** K1
        chanGraph.getNode("Lc_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_i_k1_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Chan_to_k1_scatter_loop_k1")->subgraphId = 2;
        chanGraph.getNode("Lse_tmp_i1")->subgraphId = 2;
        chanGraph.getNode("Lse_tmp_j1")->subgraphId = 2;
        chanGraph.getNode("Chan_tmp1_cmp")->subgraphId = 2;
        chanGraph.getNode("Chan_k1_cond")->subgraphId = 2;
        // True
        chanGraph.getNode("Chan_to1_update")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_k1_true")->subgraphId = 2;
        // False
        chanGraph.getNode("Chan_i1_update")->subgraphId = 2;
        chanGraph.getNode("Lse_a_update_k1_false")->subgraphId = 2;

        chanGraph.getNode("Chan_br_merge_tmp1")->subgraphId = 2;
        //**

        chanGraph.getNode("Chan_br_merge_tmp1_shadow")->subgraphId = 4;

        chanGraph.getNode("Chan_to_k2")->subgraphId = 3;
        chanGraph.getNode("Chan_i_k2")->subgraphId = 3;
        //** K2
        chanGraph.getNode("Lc_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_i_k2_scatter_loop_k2")->subgraphId = 4;
        chanGraph.getNode("Chan_to_k2_scatter_loop_k2")->subgraphId = 4;
        chanGraph.getNode("Lse_tmp_i2")->subgraphId = 4;
        chanGraph.getNode("Lse_tmp_j2")->subgraphId = 4;
        chanGraph.getNode("Chan_tmp2_cmp")->subgraphId = 4;
        chanGraph.getNode("Chan_k2_cond")->subgraphId = 4;

        chanGraph.getNode("Chan_to2_update")->subgraphId = 4;
        chanGraph.getNode("Lse_a_update_k2_true")->subgraphId = 4;

        chanGraph.getNode("Chan_i2_update")->subgraphId = 4;
        chanGraph.getNode("Lse_a_update_k2_false")->subgraphId = 4;

        chanGraph.getNode("Chan_br_merge_tmp2")->subgraphId = 4;
        //**
        chanGraph.getNode("Chan_br_merge_tmp2_shadow")->subgraphId = 4;
        chanGraph.getNode("Chan_br_merge_to")->subgraphId = 4;

        break;
    }
    case 6:
    {


        break;
    }
    case 7:
    {


        break;
    }
    default:
        Debug::throwError("Not define this subgraph number!", __FILE__, __LINE__);
    }
}