NAME = library

CC := g++
RM ?= @rm
MKDIR ?= @mkdir

CFLAGS := -std=c++1z -Wall -Wextra -fopenmp

SRC_DIR = src
BUILD_DIR = build
DATA_DIR = data
INCLUDES = inc


OBJECTS = $(NAME).o

all: $(BUILD_DIR) $(NAME) $(NAME).so
	@echo "Built $(NAME)"

debug: CFLAGS += -DDEBUG -g
debug: all

$(DATA_DIR):
	@echo "Creating data directory: $(DATA_DIR)"
	$(MKDIR) $(DATA_DIR)

$(BUILD_DIR):
	@echo "Creating build directory: $(BUILD_DIR)"
	$(MKDIR) $(BUILD_DIR)
	$(MKDIR) $(BUILD_DIR)/src

$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -fPIC -I$(INCLUDES) -c -o $@ $< -lstdc++

$(NAME): $(foreach object,$(OBJECTS),$(BUILD_DIR)/src/$(object))
	@echo "Linking $(NAME)"
	$(CC) $(CFLAGS) -o build/$@ $^

$(NAME).so: $(foreach object,$(OBJECTS),$(BUILD_DIR)/src/$(object))
	@echo "Linking $(NAME)"
	$(CC) $(CFLAGS) -fPIC -shared -o build/$@ $^ -lstdc++

BENCHMARK_DATA:= $(shell date '+%Y-%m-%d-%H:%M:%S')
SMALL_BENCH_ELEMENTS:=1000
LARGE_BENCH_ELEMENTS:=1000000

bench:
	@echo "Running large benchmark ..."
	@python benchmark.py -d $(BENCHMARK_DATA) -p split_50_50 -i 10 -s large -l false -e $(LARGE_BENCH_ELEMENTS)
	@python benchmark.py -d $(BENCHMARK_DATA) -p split_50_50 -i 10 -s large -l true -e $(LARGE_BENCH_ELEMENTS)
	@python benchmark.py -d $(BENCHMARK_DATA) -p produce_and_consume -i 10 -s large -l false -e $(LARGE_BENCH_ELEMENTS)
	@python benchmark.py -d $(BENCHMARK_DATA) -p produce_and_consume -i 10 -s large -l true -e $(LARGE_BENCH_ELEMENTS)

small-bench: $(BUILD_DIR) $(NAME).so $(DATA_DIR)
	@echo "Running small-bench ..."
	@python benchmark.py -d $(BENCHMARK_DATA) -p split_50_50 -i 3 -s small -l false -e $(SMALL_BENCH_ELEMENTS)
	@python benchmark.py -d $(BENCHMARK_DATA) -p split_50_50 -i 3 -s small -l true -e $(SMALL_BENCH_ELEMENTS)
	@python benchmark.py -d $(BENCHMARK_DATA) -p produce_and_consume -i 3 -s small -l false -e $(SMALL_BENCH_ELEMENTS)
	@python benchmark.py -d $(BENCHMARK_DATA) -p produce_and_consume -i 3 -s small -l true -e $(SMALL_BENCH_ELEMENTS)

small-plot: 
	@echo "Plotting small-bench results ..."
	bash -c 'cd plots && pdflatex "\newcommand{\DATAPATH}{../data/$$(ls ../data/ | sort -r | head -n 1)}\input{avg_plot.tex}"'
	@echo "============================================"
	@echo "Created plots/avgplot.pdf"

report: small-plot
	@echo "Compiling report ..."
	bash -c 'cd report && pdflatex report.tex'
	@echo "============================================"
	@echo "Done"

zip:
	@zip framework.zip benchmark.py Makefile README src/* plots/avg_plot.tex report/report.tex

clean:
	@echo "Cleaning build directory: $(BUILD_DIR) and binaries: $(NAME) $(NAME).so"
	$(RM) -Rf $(BUILD_DIR)
	$(RM) -f $(NAME) $(NAME).so

.PHONY: clean report
