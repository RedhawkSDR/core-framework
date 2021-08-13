#!/usr/bin/python
# Author: Thomas Goodwin <btgoodwin@geontech.com>

import urllib2, json, os, sys, re

def download_asset(path, url):
    asset_path = None
    try:
        file_name = os.path.basename(url)
        asset_path = os.path.join(path, file_name)

        if os.path.exists(asset_path):
            # Skip downloading
            asset_path = None
        else:
            if not os.path.exists(path):
                os.makedirs(path)

            f = urllib2.urlopen(url)
            with open(asset_path, "wb") as local_file:
                local_file.write(f.read())

    except Exception as e:
        sys.exit('Failed to fetch IDE. Error: {0}'.format(e))
    finally:
        return asset_path

def handle_release_assets(assets):
    assets = [ asset for asset in assets if re.match(r'redhawk-ide.+?(?=x86_64)', asset['name'])]
    if not assets:
        sys.exit('Failed to find the IDE asset')
    elif len(assets) > 1:
        sys.exit('Found too many IDE assets matching that description...?')
    return download_asset('downloads', assets[0]['browser_download_url'])

def run(pv):
    RELEASES_URL = 'http://api.github.com/repos/RedhawkSDR/redhawk/releases'
    ide_asset = ''
    try:
        releases = json.loads(urllib2.urlopen(RELEASES_URL).read())
        releases = [r for r in releases if r['tag_name'] == pv]
        if releases:
            ide_asset = handle_release_assets(releases[0]['assets'])
        else:
            sys.exit('Failed to find the release: {0}'.format(pv))

    finally:
        return ide_asset

if __name__ == '__main__':
    # First argument is the version
    asset = run(sys.argv[1])
    print asset