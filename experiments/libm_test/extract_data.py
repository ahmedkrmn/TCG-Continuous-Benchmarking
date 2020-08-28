from os import listdir
from collections import OrderedDict
from xlutils.copy import copy
from xlrd import open_workbook
from xlwt import easyxf

########################## READING TARGETS DATA ##########################

# Declare data ordered dictionary to store the extracted data
data = OrderedDict()

# Read all output file locations produced by running benchmark.sh
output_files = sorted(listdir('output'))

# Loop on all output files (targets)
for output_file in output_files:
    with open('output/' + output_file, 'r') as file:

        # Create a new ordered dictionary for the data in the target output
        target_data = OrderedDict()

        for line in file:
            line_data = line.split()

            # Extract and add function name and estimated time
            # Double check that the line is valid
            if(len(line_data) == 11):
                target_data[(line_data[3])[:-3]] = line_data[6]

        # Add target name and target_data
        data[output_file.split('-')[-1].split('.')[0]] = target_data

##########################################################################

######################## SAVING DATA TO SPREADSHEET ######################

# Copy the "domains" sheet into a new book
domains = open_workbook('function_domains.xls', formatting_info=True)
book = copy(domains)

# Add a new sheet for measurement results
measurements_sheet = book.add_sheet('measurements')

# Set measurements_sheet as default
book.active_sheet = 1

# Freeze the header row and column
measurements_sheet.set_panes_frozen(True)
measurements_sheet.set_horz_split_pos(1)
measurements_sheet.set_vert_split_pos(1)

# Bold font style
bold_style = easyxf('font: bold on')

# Loop the data ordered dictionary
for (target_index, (target_name, target_data)) in \
        enumerate(data.items()):

    # Write the target name to the measurements_sheet
    measurements_sheet.write(
        target_index + 1, 0, target_name, bold_style)

    # Loop the target data
    for (function_index, (function_name, function_value)) in \
            enumerate(target_data.items()):

        # If this is the fist target,
        # write the function names to the first row of the measurements_sheet
        if target_index == 0:
            measurements_sheet.write(
                0, function_index + 1, function_name, bold_style)

        # Write the target function estimated times to the measurements_sheet
        measurements_sheet.write(
            target_index + 1, function_index + 1, function_value)

# Save spreadsheet
book.save('math-measurements.ods')

##########################################################################
