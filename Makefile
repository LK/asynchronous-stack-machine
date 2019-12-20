all: chp act sim count

chp: processor.chp
	./generators/chp2act processor.chp > processor.act

act: processor.act
	aflat testing/processor/test_processor.act > testing/processor/test_processor.prs

sim: processor.act
	prs2sim testing/processor/test_processor.act testing/processor/test_processor

count: testing/processor/test_processor.sim
	wc -l testing/processor/test_processor.sim
