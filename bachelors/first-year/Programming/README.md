# National Higher Education Admission System

Project developed for the Programming course of the Bachelor's Degree in
Electrical and Computer Engineering at Instituto Superior Técnico.

## Project Objective

The objective of this project was to implement, in C, a simplified version of the
Portuguese National Higher Education Admission System.

The program reads information about higher education courses and candidates from
CSV files and determines the placement of each candidate according to:

- Application grade
- Number of available vacancies
- Candidate course preferences
- Minimum admission grade of each course
- Tie rules between candidates

Each candidate can apply to up to five courses, ordered by preference.

## Placement Algorithm

Candidates are processed sequentially.

For each candidate, the program attempts to place them in their highest-priority
course option.

If the course still has available vacancies, the candidate is inserted directly.

If the course is already full, the candidate's application grade is compared with
the current minimum admission grade.

If the new candidate has a higher grade, the candidate is inserted into the course.
This may cause previously placed candidates with lower grades to be removed and
reconsidered for their next course preferences.

If two or more candidates have the same grade at the admission threshold, the tie
is preserved according to the project rules, meaning that the number of admitted
students may exceed the original number of vacancies.

Candidates who cannot be placed in any of their selected courses are stored in a
separate list.

## Implementation

The program was divided into multiple source files according to responsibility.

### Data Structures

Three main structures were implemented:

- `cursos` — stores course information such as institution, course identifier,
  number of vacancies, placed candidates and minimum admission grade.
- `candidatos` — stores candidate information, application grade and up to five
  course preferences.
- `nao_colocados` — stores candidates who could not be placed.

Dynamic memory allocation is used throughout the program to store courses,
candidates and placement lists.

## Input Processing

The program reads two CSV input files:

- A course file containing institution, course, degree and vacancy information.
- A candidate file containing candidate grades and up to five course preferences.

The input is parsed and stored in dynamically allocated structures.

The user can also specify alternative input files using command-line options.

## Candidate Placement

The main placement process is implemented in `main.c`.

The program maintains a list of candidates that are still waiting to be placed.

For each candidate:

1. The highest-priority remaining course is selected.
2. The program checks the number of available vacancies.
3. If a vacancy exists, the candidate is inserted.
4. If the course is full, the candidate's grade is compared with the current
   minimum admission grade.
5. Candidate lists are reordered after insertions.
6. The minimum admission grade is recalculated.
7. Candidates removed from a course can be reconsidered for their next preference.
8. Candidates with no remaining options are added to the non-placed list.

## Candidate Ordering

Placed candidates are sorted according to their application grade.

The implementation also handles candidates with equal grades according to the
ordering rules required by the project.

Sorting is implemented explicitly in the project code.

## Output Files

The program generates four output files:

- `CNAES_Colocacoes.csv`  
  List of placed candidates.

- `CNAES_Completo.csv`  
  Complete information for each course together with its placed candidates.

- `CNAES_Cursos.csv`  
  Summary of each course, including vacancies, number of placed candidates and
  minimum admission grade.

- `CNAES_NC.csv`  
  List of candidates who were not placed.

## Command-Line Interface

The executable supports several command-line options:

```text
-h          Display help
-v value    Override the number of vacancies per course
-n value    Limit the number of candidates processed
-i file     Select the course input file
-c file     Select the candidate input file
-o file     Select the placements output file
-u file     Select the complete output file
-m file     Select the course summary output file
-x file     Select the non-placed candidates output file