NAME = library

CC := g++
RM ?= @rm
MKDIR ?= @mkdir

CFLAGS := -std=c++1z -Wall -Wextra -fopenmp -g

SRC_DIR = src
BUILD_DIR = build
DATA_DIR = data
INCLUDES = inc

OBJECTS = $(NAME).o


all: $(BUILD_DIR) $(NAME) $(NAME).so
	@echo "Built $(NAME)"

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

bench:
	@echo "This could run a sophisticated benchmark"

small-bench: $(BUILD_DIR) $(NAME).so $(DATA_DIR)
	@echo "Running small-bench ..."
	@python benchmark.py

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
