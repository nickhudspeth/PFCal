import npyscreen
import os
from subprocess import Popen, PIPE

s = Popen(["git","tag"], stdout=PIPE).communicate()[0]
vtopts = s.splitlines()

gdict = {'n_runs': -1,
        'n_events': -1,
        'use_p_gun': -1,
        'particle_type': "none",
        'sb_queue': "none",
        'lb_queue': "none",
        'git_tag': "none",
        'run_stats': -1,
        'sim_type': -1,
        'detector_model': -1,
        'detector_version': -1,
        'eos_path': "none",
        'out_dir': "none",
        'inc_angle': -999,
        'b_field': -1,
        'input_file': "none",
        'batch_job': -1}

def defineGlobals():
    # Terminal command for current configuration
    global term_cmd
    term_cmd = ''

def UpdateTerminalCommand():
    "Updates the terminal command for the currently specified configuration"
    global term_cmd
    term_cmd=''
    if(gdict['n_runs'] > 0):
        term_cmd += "for i in $(seq 1 {}); do python submitProd.py ".format(gdict['n_runs'])
        if(gdict['sb_queue'] != "none"):
            term_cmd += "-s {0} ".format(gdict['sb_queue'])
        if(gdict['lb_queue'] != "none"):
            term_cmd += "-l {0} ".format(gdict['lb_queue'])
        if(gdict['git_tag'] != "none"):
            term_cmd += "-t {0} ".format(gdict['git_tag'])
        if(gdict['use_p_gun'] > 0):
            term_cmd += "-g "
        term_cmd += "-r ${i} "
        if(gdict['detector_version'] > 0):
            term_cmd += "-v {0} ".format(gdict['detector_version'])
        if(gdict['detector_model'] > 0):
            term_cmd += "-m {0} ".format(gdict['detector_model'])
        if(gdict['eos_path'] != "none"):
            term_cmd += "-e {0} ".format(gdict['eos_path'])
        if(gdict['out_dir'] != "none"):
            term_cmd += "-o {0} ".format(gdict['out_dir'])
        if(gdict['inc_angle'] > 0):
            term_cmd += "-a {0} ".format(gdict['inc_angle'])
        if(gdict['b_field'] > 0):
            term_cmd +- "-b {0} ".format(gdict['b_field'])
        if(gdict['n_events'] > 0):
            term_cmd += "-n {0} ".format(gdict['n_events'])
    return term_cmd

class App(npyscreen.NPSAppManaged):
    def onStart(self):
        self.registerForm("MAIN", MainForm())
        self.registerForm("GENSIMEV", GenerateSimEventsForm())
        self.registerForm("GENRECOEV", GenerateRecoEventsForm())

class MainForm(npyscreen.Form):
    def create(self):
        name = "HGCAL Simulation Configuration Utility"
        global gdict;
        n_runs = self.add(npyscreen.TitleText,
                name="Number of Runs (default 5)",
                value = "5", use_two_lines=False, begin_entry_at=50)
        gdict['n_runs'] = n_runs.value
        n_events = self.add(npyscreen.TitleText,
                name= "Number of Events (default 1000)",
                value="1000", use_two_lines=False, begin_entry_at = 50)
        use_p_gun = self.add(npyscreen.TitleSelectOne, max_height = 3,
                name="Use Particle Gun",
                values = ["True", "False"], value = 0,
                scroll_exit = True, begin_entry_at = 50)
        particle_type = self.add(npyscreen.TitleText,
                name = "Particle Type (default e-)",
                value = "e-", use_two_lines=False, begin_entry_at = 50)
        simType = self.add(npyscreen.TitleSelectOne, max_height = 3,
                name="Generated Data Type",
                values = ["Generate Sim Events", "Generate Reco Events"],
                scroll_exit = True, begin_entry_at = 50)
        self.add(npyscreen.TitleFixedText,
                name = "Terminal Command for Current Configuration:",
                value = UpdateTerminalCommand())
        def afterEditing(self):
            if self.simType == 0:
                self.parentApp.setNextForm("GENSIMEV")
            else:
                self.parentApp.setNextForm("GENRECOEV")
            #    self.parentApp.setNextForm(None)

class GenerateSimEventsForm(npyscreen.Form):
    def create(self):
        self.add(npyscreen.TitleText, name = "Configure Gen-Level Simulation")
    def afterEditing(self):
        self.parentApp.setNextForm(None)

class GenerateRecoEventsForm(npyscreen.Form):
    def create(self):
        os.chdir("./userlib")
        self.add(npyscreen.TitleText, name = "Configure Reco-Level Simulation")

    def afterEditing(self):
        self.parentApp.setNextForm(None)
if __name__ == '__main__':
    defineGlobals()
    TA =App()
    TA.run()
