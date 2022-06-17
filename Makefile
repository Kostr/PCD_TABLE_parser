parse_pcd_db: parse_pcd_db.o guids.o
	gcc $^ -o $@

.PHONY: clean

clean:
	rm -f *.o parse_pcd_db
