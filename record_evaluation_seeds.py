import os
import sys
import ecole # RNG
sys.path.append(os.path.abspath(".."))
from utilities import log2

if __name__ == '__main__':

    f = "evaluation-seed-list.csv"

    if os.path.exists(f):
        os.remove(f)

    for s in range(5):
        env = ecole.environment.Branching()

        env.seed(s)

        _, _, _, _, _ = env.reset("../basic_lp.lp")

        model = env.model.as_pyscipopt()

        s1 = model.getParam("randomization/permutationseed")
        s2 = model.getParam("randomization/randomseedshift")
        s3 = model.getParam("randomization/lpseed")

        line = f"{s},{s1},{s2},{s3}"

        log2(line, f)
