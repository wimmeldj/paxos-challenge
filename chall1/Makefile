HOST    = 8080
PORTMAP = $(HOST)\:8080

IMG     = wimmeldj/paxos-chall1
V       = 1.0.0
FULL    = $(IMG)\:$(V)


container: $(FULL)
	docker create -p $(PORTMAP) --name wimmeldj $(FULL)


$(FULL):
	docker build -t $@ .

clean:
	docker container rm wimmeldj
	docker image rm $(FULL)
