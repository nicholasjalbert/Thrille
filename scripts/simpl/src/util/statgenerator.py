# Generates summary statistics for our experiments
# with Tinertia

import sys
import os
from math import sqrt, pow
from decimal import *

class Run:
    def __init__(self, result_list = []):
        if len(result_list) > 0:
            self.extract_results(result_list)

    def extract_int(self, id, item):
        assert id in item
        val = item.split(":")[1]
        val = val.strip()
        return int(val)

    def extract_float(self, id, item):
        assert id in item
        val = item.split(":")[1]
        val = val.strip()
        return float(val)

    def extract_results(self, result_list):
        assert len(result_list) >= 48
        self.id = result_list.pop(0).split()[-1]
        self.err = result_list.pop(0)
        assert "Error:" in self.err
        result_list.pop(0)
        assert "Start Schedule:" in result_list.pop(0)

        tmp = self.extract_int("Total Schedule Points", result_list.pop(0))
        self.start_size = tmp
        tmp = self.extract_int("Total Threads", result_list.pop(0))
        self.start_threads = tmp
        tmp = self.extract_int("Context Switches", result_list.pop(0))
        self.start_ctxt = tmp
        tmp = self.extract_int("Non-Preemptive", result_list.pop(0))
        self.start_non = tmp
        tmp = self.extract_int("Preemptions", result_list.pop(0))
        self.start_pre = tmp

        result_list.pop(0)
        result_list.pop(0)
        assert "Simplified Schedule" in result_list.pop(0)
        
        tmp = self.extract_int("Total Schedule Points", result_list.pop(0))
        self.end_size= tmp
        tmp = self.extract_int("Total Threads", result_list.pop(0))
        self.end_threads= tmp
        tmp = self.extract_int("Context Switches", result_list.pop(0))
        self.end_ctxt = tmp
        tmp = self.extract_int("Non-Preemptive", result_list.pop(0))
        self.end_non = tmp
        tmp = self.extract_int("Preemptions", result_list.pop(0))
        self.end_pre = tmp
   
        result_list.pop(0)

        tmp = self.extract_int("Number of Iterations", result_list.pop(0))
        self.iters = tmp
        tmp = self.extract_int("Number of Executions", result_list.pop(0))
        self.execs = tmp
        tmp = self.extract_float("Time", result_list.pop(0))
        self.time = tmp
        tmp = self.extract_int("Number of empty sched", result_list.pop(0))
        self.empty_traces = tmp
        tmp = self.extract_int("show our bug", result_list.pop(0))
        self.buggy_traces = tmp
        tmp = self.extract_int("show a different bug", result_list.pop(0))
        self.different_bug_traces = tmp
        tmp = self.extract_int("show no bug", result_list.pop(0))
        self.no_bug_traces = tmp
            
        result_list.pop(0)

        tmp = self.extract_float("all context switches", result_list.pop(0))
        self.avg_enabled_ctxt = tmp
        tmp = self.extract_float("non-preemptive", result_list.pop(0))
        self.avg_enabled_npcs = tmp
        tmp = self.extract_float("preemptive", result_list.pop(0))
        self.avg_enabled_pre = tmp
        tmp = self.extract_float("all scheduling", result_list.pop(0))
        self.avg_enabled_all = tmp
            
        result_list.pop(0)
        
        tmp = self.extract_int("Backward Consolidation s:", result_list.pop(0))
        self.backc_s = tmp
        tmp = self.extract_int("Backward Consolidation t:", result_list.pop(0))
        self.backc_t = tmp
        tmp = self.extract_int("Backward Consolidation c:", result_list.pop(0))
        self.backc_c = tmp
        tmp = self.extract_int("Backward Consolidation n:", result_list.pop(0))
        self.backc_n = tmp
        tmp = self.extract_int("Backward Consolidation p:", result_list.pop(0))
        self.backc_p = tmp

        result_list.pop(0) 

        tmp = self.extract_int("Block Removal s:", result_list.pop(0))
        self.blockr_s = tmp
        tmp = self.extract_int("Block Removal t:", result_list.pop(0))
        self.blockr_t = tmp
        tmp = self.extract_int("Block Removal c:", result_list.pop(0))
        self.blockr_c = tmp
        tmp = self.extract_int("Block Removal n:", result_list.pop(0))
        self.blockr_n = tmp
        tmp = self.extract_int("Block Removal p:", result_list.pop(0))
        self.blockr_p = tmp

        result_list.pop(0)

        tmp = self.extract_int("Forward Consolidation2 s:", result_list.pop(0))
        self.forwc_s = tmp
        tmp = self.extract_int("Forward Consolidation2 t:", result_list.pop(0))
        self.forwc_t = tmp
        tmp = self.extract_int("Forward Consolidation2 c:", result_list.pop(0))
        self.forwc_c = tmp
        tmp = self.extract_int("Forward Consolidation2 n:", result_list.pop(0))
        self.forwc_n = tmp
        tmp = self.extract_int("Forward Consolidation2 p:", result_list.pop(0))
        self.forwc_p = tmp


    def get_effect_ctxt_remove(self):
        assert self.blockr_c >= 0
        total_removed = self.backc_c + self.blockr_c + self.forwc_c
        assert self.start_ctxt == self.end_ctxt + total_removed
        if self.blockr_c == 0:
            return 1
        return (float(self.blockr_c)/total_removed)

    def get_effect_ctxt_up(self):
        assert self.forwc_c >= 0
        total_removed = self.backc_c + self.blockr_c + self.forwc_c
        assert self.start_ctxt == self.end_ctxt + total_removed
        if self.forwc_c == 0:
            return 1
        return (float(self.forwc_c)/total_removed)

    def get_effect_ctxt_down(self):
        assert self.backc_c >= 0
        total_removed = self.backc_c + self.blockr_c + self.forwc_c
        assert self.start_ctxt == self.end_ctxt + total_removed
        if self.backc_c == 0:
            return 1
        return (float(self.backc_c)/total_removed)



class Experiment:
    def __init__(self, summary_path = ""):
        self.summary_path = summary_path
        if self.summary_path == "":
            return
        self.summary_path = os.path.abspath(self.summary_path)
        self.summary_path = os.path.normpath(self.summary_path)
        self.exp_dir, tmp = os.path.split(self.summary_path)
        tmp1, tmp2 = os.path.split(self.exp_dir)
        tmp1, tmp2 = os.path.split(tmp1)
        tmp1, self.exp_name = os.path.split(tmp1)
        self.rounded_digits = 1

        self.err_dir = os.path.join(self.exp_dir, "error")
        self.runs = []

        assert os.path.exists(self.summary_path)
        assert os.path.exists(self.exp_dir)
        assert os.path.exists(self.err_dir)
        self.parse_experiments()
        getcontext().prec = 4 * len(self.runs)
        #print getcontext().prec

    def parse_experiments(self):
        experiment_summary = open(self.summary_path, "r").readlines()
        while len(experiment_summary) > 0:
            if "**RUN" in experiment_summary[0]:
                assert len(experiment_summary) >= 47
                tmp = Run(experiment_summary[:48])
                self.runs.append(tmp)
                experiment_summary = experiment_summary[48:]
            else:
                experiment_summary.pop(0)

    def get_avg_start_size(self):
        result = reduce(lambda x, y: x + y.start_size, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_start_threads(self):
        result = reduce(lambda x, y: x + y.start_threads, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_start_ctxt(self):
        result = reduce(lambda x, y: x + y.start_ctxt, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_start_non(self):
        result = reduce(lambda x, y: x + y.start_non, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_start_pre(self):
        result = reduce(lambda x, y: x + y.start_pre, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_end_size(self):
        result = reduce(lambda x, y: x + y.end_size, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_end_threads(self):
        result = reduce(lambda x, y: x + y.end_threads, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_end_ctxt(self):
        result = reduce(lambda x, y: x + y.end_ctxt, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_end_non(self):
        result = reduce(lambda x, y: x + y.end_non, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_end_pre(self):
        result = reduce(lambda x, y: x + y.end_pre, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_iterations(self):
        result = reduce(lambda x, y: x + y.iters, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_executions(self):
        result = reduce(lambda x, y: x + y.execs, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_time(self):
        result = reduce(lambda x, y: x + y.time, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_empty_traces(self):
        result = reduce(lambda x, y: x + y.empty_traces, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_buggy_traces(self):
        result = reduce(lambda x, y: x + y.buggy_traces, self.runs, 0)
        return (float(result)/len(self.runs))
    
    def get_avg_different_bug_traces(self):
        result = reduce(lambda x, y: x + y.different_bug_traces, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_no_bug_traces(self):
        result = reduce(lambda x, y: x + y.no_bug_traces, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_enabled_ctxt_switch(self):
        result = reduce(lambda x, y: x + y.avg_enabled_ctxt, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_enabled_npcs(self):
        result = reduce(lambda x, y: x + y.avg_enabled_npcs, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_enabled_pre(self):
        result = reduce(lambda x, y: x + y.avg_enabled_pre, self.runs, 0)
        return (float(result)/len(self.runs))

    def get_avg_enabled_all(self):
        result = reduce(lambda x, y: x + y.avg_enabled_all, self.runs, 0)
        return (float(result)/len(self.runs))


    #def get_avg_effect_ctxt_remove(self):
    #    tot = reduce(lambda x,y: x * y.get_effect_ctxt_remove(), self.runs, 1)
    #    return float(pow(tot, 1.0/len(self.runs)))

    #def get_avg_effect_ctxt_up(self):
    #    tot = reduce(lambda x, y: x * y.get_effect_ctxt_up(), self.runs, 1)
    #    return float(pow(tot, 1.0/len(self.runs)))

    #def get_avg_effect_ctxt_down(self):
    #    tot = reduce(lambda x, y: x * y.get_effect_ctxt_down(), self.runs, 1)
    #    return float(pow(tot, 1.0/len(self.runs)))



    def get_avg_effect_ctxt_remove(self):
        # average ctxt removed by interval removal
        tot = reduce(lambda x,y: x + y.blockr_c, self.runs, 0)
        br_effect = float(tot)/len(self.runs)

        # average ctxt removed by all operations
        overall_effect = self.get_avg_start_ctxt() - self.get_avg_end_ctxt()

        # average contribution of interval removal
        return (br_effect/overall_effect)


    def get_avg_effect_ctxt_up(self):
        # average ctxt removed by consolidate up (forward)
        tot = reduce(lambda x,y: x + y.forwc_c, self.runs, 0)
        forw_effect = float(tot)/len(self.runs)
        
        # average ctxt removed by all operations
        overall_effect = self.get_avg_start_ctxt() - self.get_avg_end_ctxt()

        # average contribution of consolidate up (forward) 
        return (forw_effect/overall_effect)


    def get_avg_effect_ctxt_down(self):
        # average ctxt removed by consolidate down (backward)
        tot = reduce(lambda x,y: x + y.backc_c, self.runs, 0)
        back_effect = float(tot)/len(self.runs)
        
        # average ctxt removed by all operations
        overall_effect = self.get_avg_start_ctxt() - self.get_avg_end_ctxt()

        # average contribution of consolidate down (backward) 
        return (back_effect/overall_effect)

    def get_percent_reduction(self, start, end):
        delta = float(start - end)
        div = delta/start
        pct = 100 * div
        return round(pct, 3)

    def get_preduction_size(self):
        start_size = self.get_avg_start_size()
        end_size = self.get_avg_end_size()
        return self.get_percent_reduction(start_size, end_size)

    def get_preduction_threads(self):
        start_threads = self.get_avg_start_threads()
        end_threads = self.get_avg_end_threads()
        return self.get_percent_reduction(start_threads, end_threads)

    def get_preduction_ctxt(self):
        start_ctxt = self.get_avg_start_ctxt()
        end_ctxt = self.get_avg_end_ctxt()
        return self.get_percent_reduction(start_ctxt, end_ctxt)

    def get_preduction_non(self):
        start_non = self.get_avg_start_non()
        end_non = self.get_avg_end_non()
        return self.get_percent_reduction(start_non, end_non)

    def get_preduction_pre(self):
        start_pre = self.get_avg_start_pre()
        end_pre = self.get_avg_end_pre()
        return self.get_percent_reduction(start_pre, end_pre)

    def geo_mean(self, rlist):
        product = Decimal(1)
        for x in rlist:
            if x == 0:
                continue
            product = product * x

        return self.nth_root(product, len(rlist))
    
    # code taken from http://www.programmish.com/?p=24       
    def nth_root(self, num, n):
        a = Decimal(num)
        oneOverN = 1 / Decimal(n)
        nMinusOne = Decimal(n) - 1
        curVal = Decimal(num) / (Decimal(n) ** 2)
        if curVal <= Decimal("1.0"):
            curVal = Decimal("1.1")
        lastVal = 0
        while lastVal != curVal:
            lastVal = curVal
            curVal = oneOverN * ( (nMinusOne * curVal) + (a / (curVal ** nMinusOne)))
        return curVal


    def get_ratio(self, start, end):
        ratio = Decimal(end)/Decimal(start)
        return ratio


    def get_ratio_size(self):
        r_list = []
        for x in self.runs:
            r_list.append(self.get_ratio(x.start_size, x.end_size))
        return round((1 - self.geo_mean(r_list)) * 100, 3)

    def get_ratio_threads(self):
        r_list = []
        for x in self.runs:
            r_list.append(self.get_ratio(x.start_threads, x.end_threads))
        return round((1 - self.geo_mean(r_list)) * 100, 3)


    def get_ratio_ctxt(self):
        r_list = []
        for x in self.runs:
            r_list.append(self.get_ratio(x.start_ctxt, x.end_ctxt))
        return round((1 - self.geo_mean(r_list)) * 100, 3)


    def get_ratio_non(self):
        r_list = []
        for x in self.runs:
            r_list.append(self.get_ratio(x.start_non, x.end_non))
        return round((1 - self.geo_mean(r_list)) * 100, 3)


    def get_ratio_pre(self):
        r_list = []
        for x in self.runs:
            r_list.append(self.get_ratio(x.start_pre, x.end_pre))
        return round((1 - self.geo_mean(r_list)) * 100, 3)


    def output_overall_statistics(self, wr):
        print >> wr, "Start Trace:"
        print >> wr, "\tAverage Size:", self.get_avg_start_size()
        print >> wr, "\tAverage Threads:", self.get_avg_start_threads()
        print >> wr, "\tAverage Context Switches:", self.get_avg_start_ctxt() 
        tmp = self.get_avg_start_non()
        print >> wr, "\tAverage Non-Preemptive Context Switches:", tmp 
        tmp = self.get_avg_start_pre()
        print >> wr, "\tAverage Preemptive Context Switches:", tmp 
    
        print >> wr, "Simplified Trace:"
        print >> wr, "\tAverage Size:", self.get_avg_end_size()
        print >> wr, "\tAverage Threads:", self.get_avg_end_threads() 
        print >> wr, "\tAverage Context Switches:", self.get_avg_end_ctxt() 
        tmp = self.get_avg_end_non()
        print >> wr, "\tAverage Non-Preemptive Context Switches:", tmp
        tmp = self.get_avg_end_pre()
        print >> wr, "\tAverage Preemptions:", tmp

        print >> wr, "Simplification (% reduction):"
        tmp = self.get_preduction_size()
        print >> wr, "\tAverage Reduction in Size:", tmp, "%"
        tmp = self.get_preduction_threads()
        print >> wr, "\tAverage Reduction in Threads:", tmp, "%"
        tmp = self.get_preduction_ctxt()
        print >> wr, "\tAverage Reduction in Context Switches:", tmp, "%"
        tmp = self.get_preduction_non()
        print >> wr, "\tAverage Reduction in Non-Preemptive CS:", tmp, "%"
        tmp = self.get_preduction_pre()
        print >> wr, "\tAverage Reduction in Preemptions:", tmp, "%"
        
        #print >> wr, "Simplification (% reduction):"
        #tmp = self.get_ratio_size()
        #print >> wr, "\tAverage Reduction in Size:", tmp, "%"
        #tmp = self.get_ratio_threads()
        #print >> wr, "\tAverage Reduction in Threads:", tmp, "%"
        #tmp = self.get_ratio_ctxt()
        #print >> wr, "\tAverage Reduction in Context Switches:", tmp, "%"
        #tmp = self.get_ratio_non()
        #print >> wr, "\tAverage Reduction in Non-Preemptive CS:", tmp, "%"
        #tmp = self.get_ratio_pre()
        #print >> wr, "\tAverage Reduction in Preemptions:", tmp, "%"

        print >> wr, "Simplification System Statistics:" 
        tmp = self.get_avg_iterations()
        print >> wr, "\tAverage Iterations", tmp
        tmp = self.get_avg_executions()
        print >> wr, "\tAverage Executions per Run:", tmp 
        tmp = self.get_avg_time()
        print >> wr, "\tAverage Time Per Run (sec):", tmp 
        tmp = self.get_avg_empty_traces()
        print >> wr, "\tAverage short-circuited traces:", tmp 
        tmp = self.get_avg_buggy_traces()
        print >> wr, "\tAverage generated traces exhibiting our bug:", tmp 
        tmp = self.get_avg_different_bug_traces()
        print >> wr, "\tAverage generated traces show a different bug:", tmp
        tmp = self.get_avg_no_bug_traces()
        print >> wr, "\tAverage generated traces which show no bug", tmp 

        print >> wr, "Program Characteristics:"
        tmp = self.get_avg_enabled_ctxt_switch()
        print >> wr, "\tAvg threads enabled at all context switches", tmp 
        tmp = self.get_avg_enabled_npcs()
        print >> wr, "\tAvg threads enabled at non-preemptive CS", tmp
        tmp = self.get_avg_enabled_pre()
        print >> wr, "\tAvg threads enabled at preemptive CS", tmp 
        tmp = self.get_avg_enabled_all()
        print >> wr, "\tAvg threads enabled at all scheduling points", tmp 


    def output_avg_effect_ctxt_csv(self, file_name):
        fout = open(file_name, "w")

        # benchmark, removal, up, down
        fout.write(self.exp_name)
        fout.write("," + str(round(self.get_avg_effect_ctxt_remove()*100, 3)))
        fout.write("," + str(round(self.get_avg_effect_ctxt_up()*100, 3)))
        fout.write("," + str(round(self.get_avg_effect_ctxt_down()*100, 3)))
        fout.write("\n")

        fout.close()

    def output_latex_table1(self, file_name):
        fout = open(file_name, "w")
        d = " & "
        sp = "    "
       
        # size, threads, ctxt, non-pre, pre 
        sss = str(round(self.get_avg_start_size(), self.rounded_digits))
        sst = str(round(self.get_avg_start_threads(), self.rounded_digits))
        ssc = str(round(self.get_avg_start_ctxt(), self.rounded_digits))
        ssn = str(round(self.get_avg_start_non(), self.rounded_digits))
        ssp = str(round(self.get_avg_start_pre(), self.rounded_digits))
        fout.write(sp + sss + d + sst + d + ssc)
        fout.write(d + ssn + d +  ssp + d + "\n")
    
        es = str(round(self.get_avg_end_size(), self.rounded_digits))
        et = str(round(self.get_avg_end_threads(), self.rounded_digits))
        ec = str(round(self.get_avg_end_ctxt(), self.rounded_digits))
        en = str(round(self.get_avg_end_non(), self.rounded_digits))
        ep = str(round(self.get_avg_end_pre(), self.rounded_digits))
        fout.write(sp + es + d + et + d + ec + d + en + d + ep + d + "\n")

        gpr_s = str(round(self.get_preduction_size(), self.rounded_digits))
        gpr_t = str(round(self.get_preduction_threads(), self.rounded_digits))
        gpr_c = str(round(self.get_preduction_ctxt(), self.rounded_digits))
        gpr_n = str(round(self.get_preduction_non(), self.rounded_digits))
        gpr_p = str(round(self.get_preduction_pre(), self.rounded_digits))
        fout.write(sp + gpr_s)  
        fout.write(d + gpr_t + d)
        fout.write( gpr_c + d)
        fout.write( gpr_n + d)
        fout.write( gpr_p + "\\\\\n")

        fout.close()


    def output_latex_table2(self, file_name):
        fout = open(file_name, "w")
        d = " & "
        sp = "    "

        iters = str(round(self.get_avg_iterations(), self.rounded_digits))
        execs = str(round(self.get_avg_executions(), self.rounded_digits))
        time = str(round(self.get_avg_time(), self.rounded_digits))
        empty = str(round(self.get_avg_empty_traces(), self.rounded_digits))
        thebug = str(round(self.get_avg_buggy_traces(), self.rounded_digits))
        diffbug = str(round(self.get_avg_different_bug_traces(), \
                self.rounded_digits))
        nobug = str(round(self.get_avg_no_bug_traces(), self.rounded_digits))
        ac = str(round(self.get_avg_enabled_ctxt_switch(), \
                self.rounded_digits))
        an = str(round(self.get_avg_enabled_npcs(), self.rounded_digits))
        ap = str(round(self.get_avg_enabled_pre(), self.rounded_digits))
        aa = str(round(self.get_avg_enabled_all(), self.rounded_digits))

        fout.write(sp + iters + d + execs + d + time + d + "\n")
        fout.write(sp + empty + d + thebug + d) 
        fout.write(diffbug + d + nobug + d + "\n")
        fout.write(sp + ac + d + an + d + ap + d + aa + "\\\\\n")

        fout.close()

def test_statistics():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    
    assert os.path.exists(td)
    runstat = os.path.join(td, "simpl-runstat-test")
    assert os.path.exists(runstat)
    e = Experiment(runstat)

    assert e.summary_path ==  runstat
    assert e.exp_dir == td
    assert e.exp_name == "thrille"
    assert e.err_dir == os.path.join(td, "error")
    assert e.rounded_digits == 1

    assert len(e.runs) == 2
    assert e.runs[0].start_size == 100
    assert e.runs[0].start_threads == 6
    assert e.runs[0].start_ctxt == 50
    assert e.runs[0].start_non == 26
    assert e.runs[0].start_pre == 24

    assert e.runs[0].end_size == 90
    assert e.runs[0].end_threads == 5
    assert e.runs[0].end_ctxt == 40
    assert e.runs[0].end_non == 21
    assert e.runs[0].end_pre == 19

    assert e.runs[0].iters == 2
    assert e.runs[0].execs == 500
    assert e.runs[0].time == 180
    assert e.runs[0].empty_traces == 3000
    assert e.runs[0].buggy_traces == 450
    assert e.runs[0].different_bug_traces == 15
    assert e.runs[0].no_bug_traces == 6
   
    assert e.runs[0].avg_enabled_ctxt == 2.1
    assert e.runs[0].avg_enabled_npcs ==  2.2
    assert e.runs[0].avg_enabled_pre == 2.3
    assert e.runs[0].avg_enabled_all == 2.4

    assert e.runs[0].backc_s == 2
    assert e.runs[0].backc_t == 0
    assert e.runs[0].backc_c == 4
    assert e.runs[0].backc_n == 0
    assert e.runs[0].backc_p == 1
    
    assert e.runs[0].blockr_s == 3
    assert e.runs[0].blockr_t == 0
    assert e.runs[0].blockr_c == 5
    assert e.runs[0].blockr_n == 2
    assert e.runs[0].blockr_p == 2

    assert e.runs[0].forwc_s == 5
    assert e.runs[0].forwc_t == 1
    assert e.runs[0].forwc_c == 1
    assert e.runs[0].forwc_n == 3
    assert e.runs[0].forwc_p == 2

    assert e.runs[1].start_size == 102
    assert e.runs[1].start_threads == 8
    assert e.runs[1].start_ctxt == 52
    assert e.runs[1].start_non == 28
    assert e.runs[1].start_pre == 26

    assert e.runs[1].end_size == 92
    assert e.runs[1].end_threads == 6
    assert e.runs[1].end_ctxt == 42
    assert e.runs[1].end_non == 25
    assert e.runs[1].end_pre == 17

    assert e.runs[1].iters == 3
    assert e.runs[1].execs == 600
    assert e.runs[1].time == 200
    assert e.runs[1].empty_traces == 3500
    assert e.runs[1].buggy_traces == 400
    assert e.runs[1].different_bug_traces == 31
    assert e.runs[1].no_bug_traces == 8
   
    assert e.runs[1].avg_enabled_ctxt == 4.1
    assert e.runs[1].avg_enabled_npcs ==  4.2
    assert e.runs[1].avg_enabled_pre == 4.3
    assert e.runs[1].avg_enabled_all == 4.4

    assert e.runs[1].backc_s == 7
    assert e.runs[1].backc_t == 1
    assert e.runs[1].backc_c == 2
    assert e.runs[1].backc_n == 0
    assert e.runs[1].backc_p == 1
    
    assert e.runs[1].blockr_s == 2
    assert e.runs[1].blockr_t == 0
    assert e.runs[1].blockr_c == 5
    assert e.runs[1].blockr_n == 1
    assert e.runs[1].blockr_p == 2

    assert e.runs[1].forwc_s == 3
    assert e.runs[1].forwc_t == 1
    assert e.runs[1].forwc_c == 3
    assert e.runs[1].forwc_n == 2
    assert e.runs[1].forwc_p == 6
    
    assert e.get_avg_start_size() == 101
    assert e.get_avg_start_threads() == 7
    assert e.get_avg_start_ctxt() == 51
    assert e.get_avg_start_non() == 27
    assert e.get_avg_start_pre() == 25

    assert e.get_avg_end_size() == 91
    assert e.get_avg_end_threads() == 5.5
    assert e.get_avg_end_ctxt() == 41
    assert e.get_avg_end_non() == 23
    assert e.get_avg_end_pre() == 18

    assert e.get_avg_iterations() == 2.5
    assert e.get_avg_executions() == 550
    assert e.get_avg_time() == 190
    assert e.get_avg_empty_traces() == 3250
    assert e.get_avg_buggy_traces() == 425
    assert e.get_avg_different_bug_traces() == 23
    assert e.get_avg_no_bug_traces() == 7


    epsilon = .0001

    assert e.get_avg_enabled_ctxt_switch() - 3.1 < epsilon
    assert e.get_avg_enabled_ctxt_switch() - 3.1 > (-1 *epsilon)
    assert e.get_avg_enabled_npcs() - 3.2 < epsilon
    assert e.get_avg_enabled_npcs() - 3.2 > (-1 * epsilon)
    assert e.get_avg_enabled_pre() - 3.3 < epsilon
    assert e.get_avg_enabled_pre() - 3.3 > (-1 * epsilon)
    assert e.get_avg_enabled_all() - 3.4 < epsilon
    assert e.get_avg_enabled_all() - 3.4 > (-1 * epsilon)

    assert e.get_avg_effect_ctxt_remove() - .5 < epsilon
    assert e.get_avg_effect_ctxt_remove() - .5 > (-1 * epsilon)
    assert e.get_avg_effect_ctxt_up() - .2 < epsilon
    assert e.get_avg_effect_ctxt_up() - .2 > (-1 * epsilon)
    assert e.get_avg_effect_ctxt_down() - .3 < epsilon
    assert e.get_avg_effect_ctxt_down() - .3 > (-1 * epsilon)
   
    epsilon = .001

    assert e.get_preduction_size() - 9.9 < epsilon
    assert e.get_preduction_size() - 9.9 > (-1 * epsilon)
    assert e.get_preduction_threads() - 21.428 < epsilon
    assert e.get_preduction_threads() - 21.428 > (-1 * epsilon)
    assert e.get_preduction_ctxt() - 19.608 < epsilon
    assert e.get_preduction_ctxt() - 19.608 > (-1 * epsilon)
    assert e.get_preduction_non() - 14.815 < epsilon
    assert e.get_preduction_non() - 14.815 > (-1 * epsilon)
    assert e.get_preduction_pre() - 28 < epsilon
    assert e.get_preduction_pre() - 28 > (-1 * epsilon)

    return 1

def test_nth_root():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    
    assert os.path.exists(td)
    runstat = os.path.join(td, "simpl-runstat-test")
    assert os.path.exists(runstat)
    e = Experiment(runstat)
    assert e.nth_root(8, 3) == 2
    assert e.nth_root(81,4) == 3
    assert e.nth_root(Decimal(".000008"), 3) == Decimal(".02")
    return 1



def test():
    test_passed = 0
    print "statgenerator.py:"
    
    test_passed += test_statistics()
    test_passed += test_nth_root()

    print "Tests passed:", test_passed

def checkEnvironment():
    if len(sys.argv) > 1 and sys.argv[1] == "-test":
        test()
        sys.exit(0)
    if len(sys.argv) != 2:
        print "usage: python statgenerator2.py [simpl-stat file]"
        print
        print "purpose: this script parses the record of an experiment and",
        print "generates summary statistics"
        print
        sys.exit(0)

    assert os.path.exists(sys.argv[1])
    assert not (os.environ.get("THRILLE_ROOT") is None)

def main():
    checkEnvironment()
    log_file = sys.argv[1]
    e = Experiment(log_file)
    e.output_overall_statistics(sys.__stdout__)
    fout = open(os.path.join(e.exp_dir, "simp-summarystat"), "w")
    e.output_overall_statistics(fout)
    fname = os.path.join(e.exp_dir, "latexchart1.tex")
    e.output_latex_table1(fname)
    fname = os.path.join(e.exp_dir, "latexchart2.tex")
    e.output_latex_table2(fname)
    fname = os.path.join(e.exp_dir, "chart3.csv")
    e.output_avg_effect_ctxt_csv(fname)


if __name__ == "__main__":
    main()
