from fabric.api import *
from fabric.colors import *
import os


env.shell = "/bin/bash -l -i -c"

# output errors in different colors
env.colorize_errors = True

# run all commands in //
env.parallel = True

# only print a warning when a command fails
env.warn_only = True

env.user = 'student'
env.roledefs = {
    'm200s': [
        '192.168.5.100',
        '192.168.6.100',
        # '192.168.7.100',
    ],
    'mokais': [
        '192.168.1.191',
        '192.168.1.192',
        # '192.168.1.193',
    ]
}

aqua_dir = '/home/student/moos-ivp-aquaticus/'
moosivp_dir = '/home/student/moos-ivp/'

def svn_up():
    run('svn up')


def update_trunk(path):
    with cd(path):
        svn_up()


def build(path):
    with cd(path):
        run('./build.sh')


def cd_missions():
    cd(os.path.join(aqua_dir, 'missions'))


@roles('mokais', 'm200s')
@task
def build_aquaticus():
    print(green('[ ] Building aquaticus on ') + red('%(host)s' % env) + '.')
    build(aqua_dir)
    print(green('[v] Building aquaticus on ') + red('%(host)s' % env) + '.')


@roles('mokais', 'm200s')
@task
def build_moosivp():
    print(green('[ ] Building moos-ivp on ') + red('%(host)s' % env) + '.')
    build(moosivp_dir)
    print(green('[v] Building moos-ivp on ') + red('%(host)s' % env) + '.')


@roles('mokais', 'm200s')
@task
def update_all():
    print(green('[ ] Updating moos-ivp ') + red('%(host)s' % env) + '.')
    update_trunk(moosivp_dir)
    print(green('[v] Updating moos-ivp ') + red('%(host)s' % env) + '.')
    print(green('[ ] Updating aquaticus ') + red('%(host)s' % env) + '.')
    update_trunk(aqua_dir)
    print(green('[v] Updating ') + red('%(host)s' % env) + '.')


@roles('mokais')
@task
def launch_humans_v():
    with cd(os.path.join(aqua_dir, 'missions', 'aquaticus1.0', 'mokai')):
        if (env.host == '192.168.1.191'):
            run('./launch_mokai.sh -e -b &')
        elif (env.host == '192.168.1.192'):
            run('./launch_mokai.sh -f -r &')


@roles('m200s')
@task
def launch_robots_v():
    with cd(os.path.join(aqua_dir, 'missions', 'aquaticus1.0', 'm200')):
        if (env.host == '192.168.5.100'):
            run('./launch_m200.sh -e -b &')
        elif (env.host == '192.168.6.100'):
            run('./launch_m200.sh -f -r &')


@task
def launch_shoreside():
    local('cd shoreside/ && ./launch_shoreside.sh')


@task
def launch_all():
    execute(launch_humans_v)
    execute(launch_robots_v)
    execute(launch_shoreside)
