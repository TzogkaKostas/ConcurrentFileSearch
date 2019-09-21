myfind: root.c leaf.c splitter.c
	gcc root.c -o myfind -lm
	gcc leaf.c -o leaf -lm
	gcc splitter.c -o splitter -lm


clean:
	rm myfind leaf splitter
