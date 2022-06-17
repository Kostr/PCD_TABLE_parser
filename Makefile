parse_pcd_db: main.o guids.o utils.o
	gcc $^ -o $@

.PHONY: clean

clean:
	rm -f *.o parse_pcd_db
