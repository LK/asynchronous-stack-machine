all: chp act

chp: nofex_processor.chp
	./generators/chp2act nofex_processor.chp > nofex_processor.act

act: nofex_processor.act
	aflat testing/processor/test_processor.act > testing/processor/test_processor.prs
