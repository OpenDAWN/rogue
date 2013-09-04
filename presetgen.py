#!/usr/bin/python2
# coding: utf-8

import rdflib
from rdflib import Graph

NS = """@prefix atom: <http://lv2plug.in/ns/ext/atom#> .
@prefix lv2: <http://lv2plug.in/ns/lv2core#> .
@prefix pset: <http://lv2plug.in/ns/ext/presets#> .
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .
@prefix state: <http://lv2plug.in/ns/ext/state#> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .
"""

INTRO = """<http://www.github.com/timowest/rogue#%s>
  a pset:Preset ;
  lv2:appliesTo <http://www.github.com/timowest/rogue> ;
  rdfs:label "%s" ;
  lv2:port """

def base(id, conf):
  m = {}
  for (k, v) in conf.items():
    m[id+"_"+k] = v
  return m

def osc(idx, conf):
  conf = conf.copy()
  conf.update({"on":1, "level":1.0})
  return base("osc"+str(idx), conf)

def dcf(idx, conf):
  conf = conf.copy()
  conf.update({"on":1, "level":1.0})
  return base("filter"+str(idx), conf)

def lfo(idx, conf):
  conf = conf.copy()
  conf.update({"on":1})
  return base("lfo"+str(idx), conf)

def env(idx, conf):
  conf = conf.copy()
  conf.update({"on":1})
  return base("env"+str(idx), conf)

def get_defaults():
  rdf_type = rdflib.URIRef('http://www.w3.org/1999/02/22-rdf-syntax-ns#type')
  lv2_ControlPort = rdflib.URIRef('http://lv2plug.in/ns/lv2core#ControlPort')  
  lv2_default = rdflib.URIRef('http://lv2plug.in/ns/lv2core#default')
  lv2_symbol = rdflib.URIRef('http://lv2plug.in/ns/lv2core#symbol')
  g = Graph()
  g.parse("rogue.ttl", format="n3")

  result = {}
  for p in g.subjects(rdf_type, lv2_ControlPort):    
    symbol = str(g.objects(p, lv2_symbol).next())
    default = g.objects(p, lv2_default).next().toPython()
    result[symbol] = default
  return result

defaults = get_defaults()

def merge(dicts):
  result = defaults.copy()
  for d in dicts:
    result.update(d)
  return result

symbols = []

def preset(name, label, *dicts):
  content = []
  content.append(NS)
  content.append(INTRO % (name, label))

  ports = []
  for (k, v) in merge(dicts).items():
    ports.append("""    [ lv2:symbol "%s" ; pset:value %s ] """ % (k, v))

  content.append(",\n".join(ports) + " .")

  symbols.append(name)

  ttlFile = open("presets/" +name+".ttl", "w")
  ttlFile.write("\n".join(content))
  ttlFile.close()

# Leads

# Basses

# FM

# Pianos

# Organs

# Brass

# Reed

# Guitars

# Strings

# Effects

def main():  
  preset("test", "Test",
      osc(1, {"type": 1}),
      dcf(1, {"type": 2}))

  ttlFile = open("presets.ttl", "w")
  ttlFile.write("test")
  ttlFile.close()

if __name__ == "__main__":
  main()
