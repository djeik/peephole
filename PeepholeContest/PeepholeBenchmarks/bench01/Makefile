all: clean
	@$(PEEPDIR)/joosc $(JOOSCOPTS) BacktrackSolver.java SudokuSolver.java Benchmark.java

java:
	@javac BacktrackSolver.java SudokuSolver.java Benchmark.java

clean:
	@rm -rf *.class *.j *~

run:
	@java -classpath "$(PEEPDIR)/jasmin.jar;$(PEEPDIR)/bcel-5.1.jar;$(PEEPDIR)/jooslib.jar;." Benchmark < in1
